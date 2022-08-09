import pickle
import re
from string import punctuation
from collections import defaultdict
from tqdm import tqdm

from language_model import Language_model
from error_model import Error_model

orig_queries = []
fix_queries = []
orig_queries_words = []
fix_queries_words = []
all_queries = []

        
def read_data():
    with open("queries_all.txt", "r", encoding='utf-8') as f:
        lines = f.readlines()
        for line in (lines):
            query=line.lower()
            if '\t' in query:
                orig = query[:(query.index('\t'))]
                fix = query[(query.index('\t')+1):]
                orig_queries.append(orig)
                fix_queries.append(fix)

                orig_words = re.sub('[' + punctuation + ']', '', orig).split()
                fix_words = re.sub('[' + punctuation + ']', '', fix).split()
                orig_queries_words.append(orig_words)
                fix_queries_words.append(fix_words)

                words_query = re.findall(r'\w+', fix)
            else:
                words_query = re.findall(r'\w+',query)
            all_queries.append(words_query)
        
        
read_data()


language_model = Language_model()
for _, query in enumerate((all_queries)):
    language_model.add_to_dict(query)

    
language_model.calc_weights()
language_model.calc_bigram_weights()


clear_language_model = Language_model()
for _, query in enumerate((fix_queries_words)):
    clear_language_model.add_to_dict(query)

clear_language_model.calc_weights()
clear_language_model.calc_bigram_weights()


with open('./language_model.pkl', 'wb') as f:
    pickle.dump(language_model, f)

with open('./clear_language_model.pkl', 'wb') as f:
    pickle.dump(clear_language_model, f)    
    
error_model = Error_model()
    
for orig, fix in (zip(orig_queries_words, fix_queries_words)):
    for i in range(min(len(orig), len(fix))):
        error_model.fit(orig[i], fix[i])

error_model.get_weights()

with open('./error_model.pkl', 'wb') as f:
    pickle.dump(error_model, f) 
