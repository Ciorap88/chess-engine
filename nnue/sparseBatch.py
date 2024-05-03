
import ctypes
import torch
import numpy as np

class SparseBatch(ctypes.Structure):
    _fields_ = [
        ('size', ctypes.c_int),
        ('num_active_white_features', ctypes.c_int),
        ('num_active_black_features', ctypes.c_int),
        ('stm', ctypes.POINTER(ctypes.c_float)),
        ('score', ctypes.POINTER(ctypes.c_float)),
        ('result', ctypes.POINTER(ctypes.c_float)),
        ('white_features_indices', ctypes.POINTER(ctypes.c_int)),
        ('black_features_indices', ctypes.POINTER(ctypes.c_int))
    ]

    def get_tensors(self, device):
        NUM_FEATURES = 40960

        stm_t = torch.from_numpy(
            np.ctypeslib.as_array(self.stm, shape=(self.size, 1))).to(device=device, non_blocking=True)
        score_t = torch.from_numpy(
            np.ctypeslib.as_array(self.score, shape=(self.size, 1))).to(device=device, non_blocking=True)
        result_t = torch.from_numpy(
            np.ctypeslib.as_array(self.result, shape=(self.size, 1))).to(device=device, non_blocking=True)

        white_features_indices_t = torch.transpose(
            torch.from_numpy(
                np.ctypeslib.as_array(self.white_features_indices, shape=(self.num_active_white_features, 2))
            ), 0, 1).long()
        black_features_indices_t = torch.transpose(
            torch.from_numpy(
                np.ctypeslib.as_array(self.black_features_indices, shape=(self.num_active_white_features, 2))
            ), 0, 1).long()

        white_features_values_t = torch.ones(self.num_active_white_features)
        black_features_values_t = torch.ones(self.num_active_black_features)

        white_features_t = torch._sparse_coo_tensor_unsafe(
            white_features_indices_t, white_features_values_t, (self.size, NUM_FEATURES)).to(device=device, non_blocking=True)
        black_features_t = torch._sparse_coo_tensor_unsafe(
            black_features_indices_t, black_features_values_t, (self.size, NUM_FEATURES)).to(device=device, non_blocking=True)

        white_features_t._coalesced_(True)
        black_features_t._coalesced_(True)

        # Now this is what the forward() required!
        return white_features_t, black_features_t, stm_t, score_t, result_t

# Let's also tell ctypes how to understand this type.
SparseBatchPtr = ctypes.POINTER(SparseBatch)

dll = ctypes.cdll.LoadLibrary('./train.dll')

dll.CreateSparseBatch.restype = SparseBatchPtr

dll.CreateSparseBatch.argtypes = [ctypes.c_char_p]

dll.DeleteSparseBatch.argtypes = [ctypes.POINTER(SparseBatch)]