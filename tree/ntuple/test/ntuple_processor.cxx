#include "ntuple_test.hxx"

#include <ROOT/RNTupleProcessor.hxx>

#include <TMemFile.h>

TEST(RNTupleProcessor, EmptyNTuple)
{
   FileRaii fileGuard("test_ntuple_processor_empty.root");
   {
      auto model = RNTupleModel::Create();
      model->MakeField<float>("x");
      auto ntuple = RNTupleWriter::Recreate(std::move(model), "ntuple", fileGuard.GetPath());
   }

   auto proc = RNTupleProcessor::Create({"ntuple", fileGuard.GetPath()});

   int nEntries = 0;
   for ([[maybe_unused]] const auto &entry : *proc) {
      nEntries++;
   }
   EXPECT_EQ(0, nEntries);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST(RNTupleProcessor, TMemFile)
{
   TMemFile memFile("test_ntuple_processor_tmemfile.root", "RECREATE");
   {
      auto model = RNTupleModel::Create();
      auto fldX = model->MakeField<float>("x");
      auto ntuple = RNTupleWriter::Append(std::move(model), "ntuple", memFile);

      for (unsigned i = 0; i < 5; ++i) {
         *fldX = static_cast<float>(i);
         ntuple->Fill();
      }
   }

   auto proc = RNTupleProcessor::Create({"ntuple", &memFile});
   auto x = proc->GetEntry().GetPtr<float>("x");

   int nEntries = 0;
   for ([[maybe_unused]] const auto &entry : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());

      EXPECT_FLOAT_EQ(static_cast<float>(nEntries - 1), *x);
   }
   EXPECT_EQ(nEntries, 5);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST(RNTupleProcessor, TDirectory)
{
   FileRaii fileGuard("test_ntuple_processor_tdirectoryfile.root");
   {
      auto file = std::unique_ptr<TFile>(TFile::Open(fileGuard.GetPath().c_str(), "RECREATE"));
      auto dir = std::unique_ptr<TDirectory>(file->mkdir("a/b"));
      auto model = RNTupleModel::Create();
      auto fldX = model->MakeField<float>("x");
      auto ntuple = RNTupleWriter::Append(std::move(model), "ntuple", *dir);

      for (unsigned i = 0; i < 5; ++i) {
         *fldX = static_cast<float>(i);
         ntuple->Fill();
      }
   }

   auto file = std::make_unique<TFile>(fileGuard.GetPath().c_str());
   auto proc = RNTupleProcessor::Create({"a/b/ntuple", file.get()});
   auto x = proc->GetEntry().GetPtr<float>("x");

   int nEntries = 0;
   for ([[maybe_unused]] const auto &entry : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());

      EXPECT_FLOAT_EQ(static_cast<float>(nEntries - 1), *x);
   }
   EXPECT_EQ(nEntries, 5);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

class RNTupleProcessorTest : public testing::Test {
protected:
   const std::array<std::string, 3> fFileNames{"test_ntuple_processor1.root", "test_ntuple_processor2.root",
                                               "test_ntuple_processor3.root"};
   const std::array<std::string, 3> fNTupleNames{"ntuple", "ntuple_aux", "ntuple_aux"};

   void SetUp() override
   {
      {
         auto model = RNTupleModel::Create();
         auto fldI = model->MakeField<int>("i");
         auto fldX = model->MakeField<float>("x");
         auto fldY = model->MakeField<std::vector<float>>("y");
         auto ntuple = RNTupleWriter::Recreate(std::move(model), fNTupleNames[0], fFileNames[0]);

         for (unsigned i = 0; i < 5; i++) {
            *fldI = i;
            *fldX = static_cast<float>(i);
            *fldY = {static_cast<float>(i), static_cast<float>(i * 2)};
            ntuple->Fill();
         }
      }
      {
         auto model = RNTupleModel::Create();
         auto fldI = model->MakeField<int>("i");
         auto fldZ = model->MakeField<float>("z");
         auto ntuple = RNTupleWriter::Recreate(std::move(model), fNTupleNames[1], fFileNames[1]);

         for (unsigned i = 0; i < 5; ++i) {
            *fldI = i;
            *fldZ = i * 2.f;
            ntuple->Fill();
         }
      }
      // Same as above, but entries in reverse order
      {
         auto model = RNTupleModel::Create();
         auto fldI = model->MakeField<int>("i");
         auto fldZ = model->MakeField<float>("z");
         auto ntuple = RNTupleWriter::Recreate(std::move(model), fNTupleNames[2], fFileNames[2]);

         for (int i = 4; i >= 0; --i) {
            *fldI = i;
            *fldZ = i * 2.f;
            ntuple->Fill();
         }
      }
   }

   void TearDown() override
   {
      for (const auto &fileName : fFileNames) {
         std::remove(fileName.c_str());
      }
   }
};

TEST_F(RNTupleProcessorTest, Base)
{
   auto proc = RNTupleProcessor::Create({fNTupleNames[0], fFileNames[0]});

   int nEntries = 0;

   for (const auto &entry : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());

      EXPECT_FLOAT_EQ(static_cast<float>(nEntries - 1), *entry.GetPtr<float>("x"));

      std::vector<float> yExp{static_cast<float>(nEntries - 1), static_cast<float>((nEntries - 1) * 2)};
      EXPECT_EQ(yExp, *entry.GetPtr<std::vector<float>>("y"));
   }
   EXPECT_EQ(nEntries, 5);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, BaseWithModel)
{

   auto model = RNTupleModel::Create();
   auto fldX = model->MakeField<float>("x");

   auto proc = RNTupleProcessor::Create({fNTupleNames[0], fFileNames[0]}, std::move(model));

   int nEntries = 0;

   for (const auto &entry : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());

      EXPECT_FLOAT_EQ(static_cast<float>(nEntries - 1), *fldX);

      try {
         entry.GetPtr<std::vector<float>>("y");
         FAIL() << "fields not present in the model passed to the processor shouldn't be readable";
      } catch (const ROOT::RException &err) {
         EXPECT_THAT(err.what(), testing::HasSubstr("invalid field name: y"));
      }
   }
   EXPECT_EQ(nEntries, 5);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, BaseWithBareModel)
{
   auto model = RNTupleModel::CreateBare();
   model->MakeField<float>("x");

   auto proc = RNTupleProcessor::Create({fNTupleNames[0], fFileNames[0]}, std::move(model));

   EXPECT_STREQ("ntuple", proc->GetProcessorName().c_str());

   {
      auto namedProc = RNTupleProcessor::Create({fNTupleNames[0], fFileNames[0]}, "my_ntuple");
      EXPECT_STREQ("my_ntuple", namedProc->GetProcessorName().c_str());
   }

   int nEntries = 0;

   for (const auto &entry : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());

      EXPECT_FLOAT_EQ(static_cast<float>(nEntries - 1), *entry.GetPtr<float>("x"));

      try {
         entry.GetPtr<std::vector<float>>("y");
         FAIL() << "fields not present in the model passed to the processor shouldn't be readable";
      } catch (const ROOT::RException &err) {
         EXPECT_THAT(err.what(), testing::HasSubstr("invalid field name: y"));
      }
   }
   EXPECT_EQ(nEntries, 5);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, ChainedChain)
{
   std::vector<std::unique_ptr<RNTupleProcessor>> innerProcs;
   innerProcs.push_back(
      RNTupleProcessor::CreateChain({{fNTupleNames[0], fFileNames[0]}, {fNTupleNames[0], fFileNames[0]}}));
   innerProcs.push_back(RNTupleProcessor::Create({fNTupleNames[0], fFileNames[0]}));

   auto proc = RNTupleProcessor::CreateChain(std::move(innerProcs));

   int nEntries = 0;

   for (const auto &entry [[maybe_unused]] : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());
      EXPECT_EQ(*entry.GetPtr<int>("i"), proc->GetCurrentEntryNumber() % 5);
      EXPECT_EQ(static_cast<float>(*entry.GetPtr<int>("i")), *entry.GetPtr<float>("x"));
   }
   EXPECT_EQ(nEntries, 15);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, ChainedJoin)
{
   std::vector<std::unique_ptr<RNTupleProcessor>> innerProcs;
   innerProcs.push_back(
      RNTupleProcessor::CreateJoin({fNTupleNames[0], fFileNames[0]}, {{fNTupleNames[1], fFileNames[1]}}, {}));
   innerProcs.push_back(
      RNTupleProcessor::CreateJoin({fNTupleNames[0], fFileNames[0]}, {{fNTupleNames[1], fFileNames[1]}}, {}));

   auto proc = RNTupleProcessor::CreateChain(std::move(innerProcs));

   int nEntries = 0;

   auto x = proc->GetEntry().GetPtr<float>("x");

   for (const auto &entry [[maybe_unused]] : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());
      EXPECT_EQ(*entry.GetPtr<int>("i"), proc->GetCurrentEntryNumber() % 5);

      EXPECT_EQ(static_cast<float>(*entry.GetPtr<int>("i")), *x);
      EXPECT_EQ(*x * 2, *entry.GetPtr<float>("ntuple_aux.z"));
   }
   EXPECT_EQ(nEntries, 10);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, ChainedJoinUnaligned)
{
   std::vector<std::unique_ptr<RNTupleProcessor>> innerProcs;
   innerProcs.push_back(
      RNTupleProcessor::CreateJoin({fNTupleNames[0], fFileNames[0]}, {{fNTupleNames[2], fFileNames[2]}}, {"i"}));
   innerProcs.push_back(
      RNTupleProcessor::CreateJoin({fNTupleNames[0], fFileNames[0]}, {{fNTupleNames[2], fFileNames[2]}}, {"i"}));

   auto proc = RNTupleProcessor::CreateChain(std::move(innerProcs));

   int nEntries = 0;

   auto x = proc->GetEntry().GetPtr<float>("x");

   for (const auto &entry [[maybe_unused]] : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());
      EXPECT_EQ(*entry.GetPtr<int>("i"), proc->GetCurrentEntryNumber() % 5);

      EXPECT_EQ(static_cast<float>(*entry.GetPtr<int>("i")), *x);
      EXPECT_EQ(*x * 2, *entry.GetPtr<float>("ntuple_aux.z"));
   }
   EXPECT_EQ(nEntries, 10);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, JoinedChain)
{
   auto primaryChain =
      RNTupleProcessor::CreateChain({{fNTupleNames[0], fFileNames[0]}, {fNTupleNames[0], fFileNames[0]}});

   std::vector<std::unique_ptr<RNTupleProcessor>> auxiliaryChain;
   auxiliaryChain.emplace_back(
      RNTupleProcessor::CreateChain({{fNTupleNames[1], fFileNames[1]}, {fNTupleNames[1], fFileNames[1]}}));

   auto proc = RNTupleProcessor::CreateJoin(std::move(primaryChain), std::move(auxiliaryChain), {});

   int nEntries = 0;

   auto x = proc->GetEntry().GetPtr<float>("x");

   for (const auto &entry [[maybe_unused]] : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());
      EXPECT_EQ(*entry.GetPtr<int>("i"), proc->GetCurrentEntryNumber() % 5);

      EXPECT_EQ(static_cast<float>(*entry.GetPtr<int>("i")), *x);
      EXPECT_EQ(*x * 2, *entry.GetPtr<float>("ntuple_aux.z"));
   }
   EXPECT_EQ(nEntries, 10);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, JoinedChainUnaligned)
{
   auto primaryChain =
      RNTupleProcessor::CreateChain({{fNTupleNames[0], fFileNames[0]}, {fNTupleNames[0], fFileNames[0]}});

   std::vector<std::unique_ptr<RNTupleProcessor>> auxiliaryChain;
   auxiliaryChain.emplace_back(
      RNTupleProcessor::CreateChain({{fNTupleNames[2], fFileNames[2]}, {fNTupleNames[2], fFileNames[2]}}));

   auto proc = RNTupleProcessor::CreateJoin(std::move(primaryChain), std::move(auxiliaryChain), {"i"});

   int nEntries = 0;

   auto x = proc->GetEntry().GetPtr<float>("x");

   for (const auto &entry [[maybe_unused]] : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());
      EXPECT_EQ(*entry.GetPtr<int>("i"), proc->GetCurrentEntryNumber() % 5);

      EXPECT_EQ(static_cast<float>(*entry.GetPtr<int>("i")), *x);
      EXPECT_EQ(*x * 2, *entry.GetPtr<float>("ntuple_aux.z"));
   }
   EXPECT_EQ(nEntries, 10);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, JoinedJoinComposedPrimary)
{
   auto primaryProc =
      RNTupleProcessor::CreateJoin({fNTupleNames[0], fFileNames[0]}, {{fNTupleNames[1], fFileNames[1]}}, {});

   std::vector<std::unique_ptr<RNTupleProcessor>> auxProcs;
   auxProcs.emplace_back(RNTupleProcessor::Create({fNTupleNames[2], fFileNames[2]}, "ntuple_aux2"));

   auto proc = RNTupleProcessor::CreateJoin(std::move(primaryProc), std::move(auxProcs), {"i"});

   int nEntries = 0;

   auto x = proc->GetEntry().GetPtr<float>("x");

   for (const auto &entry [[maybe_unused]] : *proc) {
      EXPECT_EQ(++nEntries, proc->GetNEntriesProcessed());
      EXPECT_EQ(nEntries - 1, proc->GetCurrentEntryNumber());
      EXPECT_EQ(*entry.GetPtr<int>("i"), proc->GetCurrentEntryNumber() % 5);

      EXPECT_EQ(static_cast<float>(*entry.GetPtr<int>("i")), *x);
      EXPECT_EQ(*x * 2, *entry.GetPtr<float>("ntuple_aux.z"));
      EXPECT_EQ(*entry.GetPtr<float>("ntuple_aux.z"), *entry.GetPtr<float>("ntuple_aux2.z"));
   }
   EXPECT_EQ(nEntries, 5);
   EXPECT_EQ(nEntries, proc->GetNEntriesProcessed());
}

TEST_F(RNTupleProcessorTest, JoinedJoinComposedAuxiliary)
{
   auto primaryProc = RNTupleProcessor::Create({fNTupleNames[0], fFileNames[0]});

   std::vector<std::unique_ptr<RNTupleProcessor>> auxProcsIntermediate;
   auxProcsIntermediate.emplace_back(RNTupleProcessor::Create({fNTupleNames[2], fFileNames[2]}, "ntuple_aux2"));

   std::vector<std::unique_ptr<RNTupleProcessor>> auxProcs;
   auxProcs.emplace_back(RNTupleProcessor::CreateJoin(RNTupleProcessor::Create({fNTupleNames[1], fFileNames[1]}),
                                                      std::move(auxProcsIntermediate), {"i"}));

   try {
      auto proc = RNTupleProcessor::CreateJoin(std::move(primaryProc), std::move(auxProcs), {});
   } catch (const ROOT::RException &err) {
      EXPECT_THAT(err.what(), testing::HasSubstr("auxiliary RNTupleJoinProcessors are currently not supported"));
   }
}
