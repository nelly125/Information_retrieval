from collections import defaultdict
import functools
import numpy as np 
import math
import Levenshtein

class Error_model:

    def __init__(self):
        dict_int = functools.partial(defaultdict, int)
        dict_d = functools.partial(defaultdict, float)

        self.error_dict = defaultdict(dict_int)
        self.weights = defaultdict(dict_d)
    
    def fit(self, orig, fix):
        for operation, left, right in Levenshtein.editops(orig, fix):
#             print(operation, left, right)

            if (operation == 'replace'):
#                 print(orig[left], fix[right])
                self.error_dict[orig[left]][fix[right]] += 1
            if (operation == 'delete'):
#                 print(orig[left])
                self.error_dict[orig[left]][''] += 1
            if (operation == 'insert'):
#                 print(fix[right])
                self.error_dict[''][fix[right]] += 1
    
    def get_weights(self, alpha=0.1):
        for k in self.error_dict.keys():
            w = np.sum(list(self.error_dict[k].values()))
            for k1 in self.error_dict[k]:
                self.weights[k][k1] = np.log(self.error_dict[k][k1]/w)*(-1)

    def get_weight(self, word_1, word_2):
        error_weight = 0
        for operation, left, right in Levenshtein.editops(word_1, word_2):
            if (operation == 'replace'):
                error_weight += self.weights[word_1[left]][word_2[right]]
            if (operation == 'delete'):
                error_weight += self.weights[word_1[left]]['']
            if (operation == 'insert'):
                error_weight += self.weights[''][word_2[right]]             

