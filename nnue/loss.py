import torch
from math import log

class Loss:
    def __init__(self, scaling_factor, lambda_):
        # TODO: tune these
        self.scaling_factor = scaling_factor
        self.lambda_ = lambda_
        
    def CpToWdlEval(self, cp_eval):
        wdl_eval = torch.sigmoid(cp_eval / self.scaling_factor)
        return wdl_eval

    def ResultEvalInterpolation(self, wdl_eval, game_result):
        return self.lambda_ * wdl_eval + (1 - self.lambda_) * game_result

    def getWdlValueTarget(self, eval_target, game_result):
        wdl_eval_target = self.CpToWdlEval(eval_target)
        return self.ResultEvalInterpolation(wdl_eval_target, game_result)

    def MSE(self, pred, eval_target, game_result):    
        wdl_pred = self.CpToWdlEval(pred)
        wdl_value_target = self.getWdlValueTarget(eval_target, game_result)
        return (wdl_pred - wdl_value_target) ** 2
        
    def CrossEntropy(self, pred, eval_target, game_result):
        wdl_pred = self.CpToWdlEval(pred)
        wdl_value_target = self.getWdlValueTarget(eval_target, game_result)
        # The first term in the loss has 0 gradient, because we always
        # differentiate with respect to `wdl_pred`, but it makes the loss nice
        # in the sense that 0 is the minimum.
        epsilon = 1e-12
        return ( 
            (wdl_value_target * log(wdl_value_target + epsilon) + (1 - wdl_value_target) * log(1 - wdl_value_target + epsilon)) 
        - (wdl_value_target * log(wdl_pred   + epsilon) + (1 - wdl_value_target) * log(1 - wdl_pred   + epsilon))
        )
