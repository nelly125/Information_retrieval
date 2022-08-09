from language_model import Language_model

class Keyboard:
    def __init__(self, language_model):
        self.language_model = language_model
        self.eng_rus = {'q':'й', 'w':'ц', 'e':'у', 'r':'к', 't':'е', 'y':'н'\
           , 'u':'г', 'i':'ш', 'o':'щ', 'p':'з','[':'х',']':'ъ','a':'ф',\
           's':'ы','d':'в','f':'а','g':'п','h':'р','j':'о','k':'л'\
           ,'l':'д',';':'ж','\'':'э','z':'я','x':'ч','c':'с','v':'м','b':'и','n':'т','m':'ь',',':'б','.':'ю', '`': 'ё'}
        self.rus_eng = {rus:eng for eng, rus in self.eng_rus.items()}
        
    def word_to_rus(self, word):
        new_word = word
        for i, c in enumerate(word):
            if c in self.eng_rus:
                new_word = new_word[:i] + self.eng_rus[c] + new_word[i + 1:]
        return new_word

    def word_to_eng(self, word):
        new_word = word
        for i, c in enumerate(word):
            if c in self.rus_eng:
                new_word = new_word[:i] + self.rus_eng[c] + new_word[i + 1:]
        return new_word
    
    def change_keyboard(self, word):
        rus_word = self.word_to_rus(word)
#         print(rus_word)
        eng_word = self.word_to_eng(word)
#         print(eng_word)
        rus_word_weight = self.language_model.weights[rus_word] 
        eng_word_weight = self.language_model.weights[eng_word]
        if not rus_word_weight:
            rus_word_weight = self.language_model.min_weight
        if not eng_word_weight:
            eng_word_weight = self.language_model.min_weight
#         print(rus_word_weight, eng_word_weight)
        if (rus_word_weight > eng_word_weight and (rus_word_weight)):
            return rus_word
        elif (eng_word_weight > rus_word_weight):
            return eng_word
        else:
            return None

