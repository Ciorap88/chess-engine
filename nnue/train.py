import torch
import torch.nn as nn
import torch.optim as optim
import numpy as np
import ctypes
import random

from nnue import NNUE
from sparseBatch import dll
from loss import Loss

if __name__ == "__main__":
    loss = Loss(410, 0.75)
    
    numberOfBatches = ctypes.c_int()
    batchArr = dll.CreateSparseBatchArr("pos.txt".encode('utf-8'), 4096, ctypes.byref(numberOfBatches))
    numberOfBatches = numberOfBatches.value
    
    
    batches_t = []
    for i in range(numberOfBatches):
        print(f"Creating tensors for batch {i}/{numberOfBatches}.", end='\r')
        batches_t.append(batchArr[i].contents.get_tensors(0))
    print()
    
    model = NNUE(batches_t[0][0].size(1), 64, 64, 1).to(0)
    
    optimizer = optim.Adam(model.parameters(), lr=0.05)
    
    # Step 5: Write the training loop
    num_epochs = 10
    for epoch in range(num_epochs):
        random.shuffle(batches_t)
        for batch_idx, batch in enumerate(batches_t):
            print(f"Epoch {epoch}/{num_epochs}: batch {batch_idx}/{numberOfBatches}...", end='\r')
            
            white_features_t, black_features_t, stm_t, score_t, result_t = batch
            
            # Forward pass
            output = model(white_features_t, black_features_t, stm_t)
            
            # Compute the loss
            loss_val = loss.CrossEntropy(output, score_t, result_t)
            
            # Backward pass
            optimizer.zero_grad()
            loss_val.backward()
            optimizer.step()
        
            
        # Print progress
        print(f"\nEpoch {epoch+1}/{num_epochs} done, loss={round(loss_val.item(), 5)}")

    torch.save(model.state_dict(), 'nnue_trained.pth')
    print("Model weights saved successfully to nnue_trained.pth.")

    dll.DeleteSparseBatchArr(batchArr, numberOfBatches)


