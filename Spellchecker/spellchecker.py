import pickle
from bor import Node, Bor_tree
from language_model import Language_model
from error_model import Error_model
from search import Spellchecker
from keyboard import Keyboard
from classifier import Features, Classifier

if __name__ == '__main__':
    with open('./language_model.pkl', 'rb') as f:
        language_model = pickle.load(f)
    with open('./error_model.pkl', 'rb') as f:
        error_model = pickle.load(f)
    with open('./clear_language_model.pkl', 'rb') as f:
        clear_language_model = pickle.load(f)    
    bor = Bor_tree(language_model)
    bor.fit()
    keyboard = Keyboard(language_model)
    with open('./classifier_5.pkl', 'rb') as f:
        clf = pickle.load(f)
    spellcheck = Spellchecker(language_model, clear_language_model, bor, keyboard, clf)
    while (True):
        try:
            query =input()
        except (EOFError):
            break
        print(spellcheck.fix_query(query))
