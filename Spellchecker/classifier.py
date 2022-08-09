import re
#from catboost import CatBoostRegressor
from sklearn.model_selection import KFold
from sklearn.metrics import f1_score, accuracy_score
import random
#from catboost import CatBoostClassifier
#from xgboost import XGBClassifier
from sklearn.ensemble import GradientBoostingClassifier
#from sklearn.ensemble import GradientBoostingRegressor
from tqdm import tqdm
from string import punctuation
import numpy as np

class Features:
    
    def __init__(self, language_model):
        self.language_model = language_model
        
    def get_features(self, query, words):
        x = list()
        x.append(len(query))
        x.append(len(words)) 
#         x.append(len(re.findall(r'[^\w]', query)))
        punctuations = re.compile(r'[,.";\[\]~]:}{}<>#').findall(query)
        spaces = re.compile(r' ').findall(query)
        x.append(len(punctuations))  
        x.append(len(spaces))
        if(words):
            x.append(self.language_model.get_query_weight(words))
        else:
            x.append(0)

        count = 0
        for word in words:
            if word in self.language_model.language_dict:
                count += 1
        x.append(len(words) - count)
        
        return x

    def fit(self):
        orig_queries = []
        fix_queries = []
        orig_queries_words = []
        fix_queries_words = []
        all_queries = []
        X_train = []
        y_train = []
        with open("queries_all.txt") as f:
            lines = f.readlines()
            for line in tqdm(lines):
                if random.randint(1, 100) < 97:
                    continue
                query=line.lower()
                
                if '\t' in query:
                    orig = query[:(query.index('\t'))]
                    orig_words = re.sub('[' + punctuation + ']', '', orig).split()
                    X_train.append(self.get_features(orig, orig_words))
                    y_train.append(0)

                    fix = query[(query.index('\t')+1):]
                    fix_words = re.sub('[' + punctuation + ']', '', fix).split()
                    X_train.append(self.get_features(fix, fix_words))
                    y_train.append(1)    

                else:
                    words = re.findall(r'\w+',query)
                    X_train.append(self.get_features(query, words))
                    y_train.append(1)
        
        return X_train, y_train
class Classifier:
    
    def __init__(self):
#        self.model = CatBoostClassifier(n_estimators=175, max_depth=5, verbose = False)
         self.model = GradientBoostingClassifier(n_estimators=175, max_depth=6)
#         self.model = classifier
#        self.model = GradientBoostingRegressor(n_estimators=250, max_depth=5)
    
    def train(self, X, y):
        X = np.asarray(X)
        y = np.asarray(y)
        self.model.fit(X, y)
#         f1 = []
#         acc = []
#         kf = KFold(n_splits=5, shuffle=True)
#         for train_index, test_index in kf.split(X):
#             X_train = X[train_index]
#             y_train = y[train_index]
#             X_test = X[test_index]
#             y_test = y[test_index]
#             self.model.fit(X_train, y_train)
#             y_pred = self.model.predict(X_test)
#             f1.append(f1_score(y_test, y_pred > 0.98, pos_label=0))
#             acc.append(accuracy_score(y_test, y_pred > 0.98))

#         print(sum(f1)/len(f1))
#         print(sum(acc)/len(acc))
