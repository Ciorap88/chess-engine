import torch
import torch.nn as nn
import torch.optim as optim

from nnue import NNUE
from sparseBatch import dll
from loss import Loss

if __name__ == "__main__":
    loss = Loss(410, 0.75)
    batch = dll.CreateSparseBatch("pos.txt".encode('utf-8'))

    white_features_t, black_features_t, stm_t, score_t, result_t = batch.contents.get_tensors(0)
    
    model = NNUE(white_features_t.size(1), 64, 64, 1).to(0)
    
    optimizer = optim.Adam(model.parameters(), lr=0.05)
    
    # Step 5: Write the training loop
    num_epochs = 10
    for epoch in range(num_epochs):
        # Forward pass
        output = model(white_features_t, black_features_t, stm_t)
        
        # Compute the loss
        loss_val = loss.CrossEntropy(output, score_t, result_t)
        
        # Backward pass
        optimizer.zero_grad()
        loss_val.backward()
        optimizer.step()
        
            
        # Print progress
        print(f"Epoch [{epoch+1}/{num_epochs}], loss={round(loss_val.item(), 5)}")

    torch.save(model.state_dict(), 'nnue_trained.pth')

    dll.DeleteSparseBatch(batch)


