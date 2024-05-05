import torch
import torch.nn as nn
import torch.optim as optim
from torch.optim.lr_scheduler import ReduceLROnPlateau
import numpy as np
import ctypes
import random

from nnue import NNUE
from sparseBatch import dll
from loss import Loss

if __name__ == "__main__":
    loss = Loss(410, 0.75)

    batch_size = 512
    MAX_ENTRIES_IDX = 10000000    
    
    numberOfBatches = ctypes.c_int()
    batchArr = dll.CreateSparseBatchArr("pos.txt".encode('utf-8'), batch_size, MAX_ENTRIES_IDX, ctypes.byref(numberOfBatches))
    numberOfBatches = numberOfBatches.value
    
    
    train_batches_t = []
    num_training_batches = 5000
    for i in range(num_training_batches):
        print(f"Creating tensors for training batch {i}/{num_training_batches}.", end='\r')
        train_batches_t.append(batchArr[i].contents.get_tensors(0))
    print()
    
    val_batches_t = []
    num_val_batches = num_training_batches // 10
    for i in range(num_training_batches, num_training_batches + num_val_batches):
        print(f"Creating tensors for val batch {i - num_training_batches}/{num_val_batches}.", end='\r')
        val_batches_t.append(batchArr[i].contents.get_tensors(0))
    print()
    
    model = NNUE(train_batches_t[0][0].size(1), 16, 32, 1).to(0)
    
    optimizer = optim.Adam(model.parameters(), lr=0.005)
    scheduler = ReduceLROnPlateau(optimizer, mode='min', factor=0.1, patience=2, min_lr=1e-05)    
    
    best_loss = float('inf')
    best_model_weights = None
    no_improvement_count = 0
    patience = 10
        
    num_epochs = 50
    for epoch in range(num_epochs):
        model.train()
        random.shuffle(train_batches_t)
        total_loss = 0.0
        
        PERCENT = len(train_batches_t) // 100
        for batch_idx, batch in enumerate(train_batches_t):
            if(batch_idx % PERCENT == 0):
                print(f"Epoch {epoch+1}/{num_epochs}: batch {batch_idx}/{len(train_batches_t)}...", end='\r')
            
            white_features_t, black_features_t, stm_t, score_t, result_t = batch
            
            output = model(white_features_t, black_features_t, stm_t)
            
            loss_val = loss.CrossEntropy(output, score_t, result_t)
            total_loss += loss_val / len(train_batches_t)
            
            optimizer.zero_grad()
            loss_val.backward()
            optimizer.step()
            
        model.eval()
        random.shuffle(val_batches_t)
        val_loss = 0.0

        with torch.no_grad():
            print("\nValidating... ", end="")
            
            for batch_idx, batch in enumerate(val_batches_t):
                white_features_t, black_features_t, stm_t, score_t, result_t = batch

                output = model(white_features_t, black_features_t, stm_t)

                loss_val = loss.CrossEntropy(output, score_t, result_t)
                val_loss += loss_val / len(val_batches_t)
                
        print("done.")
        scheduler.step(val_loss)
         
        print(f"Epoch {epoch+1}/{num_epochs} done, loss={round(total_loss.item(), 5)}, val_loss={round(val_loss.item(), 5)}, lr={scheduler.get_last_lr()[0]}")
        
        if val_loss.item() < best_loss:
            best_loss = val_loss.item()
            best_model_weights = model.state_dict()
            no_improvement_count = 0
        else:
            no_improvement_count += 1
        
        if no_improvement_count >= patience:
            print(f"No improvement in val_loss for {patience} epochs. Exiting training loop.")
            break
        
            
    output_file_name = f'nnue_trained_loss{str(round(best_loss, 5)).replace(".", "")}.pth'
    torch.save(best_model_weights, output_file_name)
    print(f"Model weights saved successfully to {output_file_name}.")

    dll.DeleteSparseBatchArr(batchArr, numberOfBatches)


