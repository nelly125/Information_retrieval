import functools
from collections import defaultdict
import numpy as np

class Language_model:
    
    def __init__(self):
        dict_int = functools.partial(defaultdict, int)
        dict_float = functools.partial(defaultdict, float)

        self.bigrams_dict = defaultdict(dict_int)
        self.language_dict = defaultdict(int)
        self.char_counter = defaultdict(int)
        self.bigram_char_counter = defaultdict(dict_int)
        self.bigram_weights = defaultdict(dict_int)
        self.weights=defaultdict(float)
        self.min_weight = 100
        self.standart_weight = 1e-12
        self.size = 0
        
    def add_to_dict(self, query):
        for i, word in enumerate(query):
            self.language_dict[word] += 1
            self.size += 1
            self.add_char_bigrams(word)
        self.add_to_dict_bigrams(query)
            
    def add_to_dict_bigrams(self, query):
        for i in range(len(query)-1):
            self.bigrams_dict[query[i]][query[i+1]] += 1
        
    def add_char_bigrams(self, word):
        for c in word:
                self.char_counter[c] += 1
        for i in range(len(word) - 1):
            self.bigram_char_counter[word[i]][word[i + 1]] += 1  
            
    def calc_weights(self):
        for i, word in self.language_dict.items():
            self.weights[i] = word / self.size
            if self.weights[i] < self.min_weight:
                self.min_weight = self.weights[i]
            
    def get_word_weight(self, word):
        if word in self.language_dict:
            return self.weights[word]
        else:
            return 1e-12
        
    def get_bigram_weight(self, word_1, word_2, flag = True):
        if word_2 in self.bigrams_dict[word_1]:
            return self.bigram_weights[word_1][word_2] / self.bigram_weights[word_1][""]
        else:
            return 1e-12
        
    def get_query_weight(self, query):
        if len(query) == 1:
            if query[0] in self.language_dict:
                return self.get_word_weight(query[0])
            else:
                return self.get_word_weight(query[0]) * 10**(-len(query[0])*5)
        weight = self.get_word_weight(query[0])
        flag = True
#         count_bigram = 0
#         iterations = 0
        for i in range(len(query) - 1):
#             iterations += 1
            bigram_weight = self.get_bigram_weight(query[i], query[i + 1])
            weight *= bigram_weight
#             print(bigram_weight)
#             if bigram_weight == 1e-12:
#                 count_bigram += 1
#             if (count_bigram == iterations):
                
        return weight
            
    def get_unigram_weight(self, query):
        weight = 1
        for word in query:
            weight *= self.get_word_weight(word)
#             print(word, weight, self.get_word_weight(word))
        return weight

    def calc_bigram_weights(self):
        for i in self.bigrams_dict.keys():
            count = 0
            for j in self.bigrams_dict[i].keys():
                count += self.bigrams_dict[i][j]
            self.bigram_weights[i] = self.bigrams_dict[i]
            self.bigram_weights[i][""] = count
