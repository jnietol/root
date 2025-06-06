import unittest

import ROOT


class NTupleModel(unittest.TestCase):
    """Various tests for the RNTupleModel class"""

    def test_create_model(self):
        """A model can be created."""

        model = ROOT.RNTupleModel.Create()
        self.assertTrue(model)

    def test_create_bare_model(self):
        """A bare model can be created."""

        model = ROOT.RNTupleModel.CreateBare()
        self.assertTrue(model)

    def test_estimate_memory_usage(self):
        """Can estimate the memory usage of a model."""

        model = ROOT.RNTupleModel.CreateBare()
        model.MakeField["int"]("i")
        model.MakeField["std::vector<std::vector<float>>"]("f")

        options = ROOT.RNTupleWriteOptions()
        InitialPageSize = 16
        MaxPageSize = 100
        ClusterSize = 6789
        options.SetInitialUnzippedPageSize(InitialPageSize)
        options.SetMaxUnzippedPageSize(MaxPageSize)
        options.SetApproxZippedClusterSize(ClusterSize)

        Expected = 4 * MaxPageSize + 4 * InitialPageSize + 3 * ClusterSize
        self.assertEqual(model.EstimateWriteMemoryUsage(options), Expected)


if __name__ == "__main__":
    unittest.main()
