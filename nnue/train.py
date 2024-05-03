from sparseBatch import dll
from loss import Loss
import torch

if __name__ == "__main__":
    loss = Loss(410, 0.75)
    batch = dll.CreateSparseBatch("pos.txt".encode('utf-8'))

    print(loss.CrossEntropy(torch.tensor(-1000), torch.tensor(-1000), torch.tensor(0.0)))
    white_features_t, black_features_t, stm_t, score_t = batch.contents.get_tensors(0)

    print(stm_t[:20])
    print(white_features_t)

    dll.DeleteSparseBatch(batch)