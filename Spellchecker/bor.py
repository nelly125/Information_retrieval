class Node:
    def __init__(self):
        self.word = None
        self.children = {}

    def insert(self, word):
        node = self
        for c in word:
            if c not in node.children:
                node.children[c] = Node()
            node = node.children[c]
        node.word = word

class Bor_tree():
    def __init__(self, language_model):
        self.tree = Node()
        self.language_model = language_model
    
    def fit(self):
        for key in self.language_model.language_dict.keys():
            self.tree.insert(key)
    
    def search(self, word, lev_dist = 1):
        variants = []
        row = range(1 + len(word))
        for c in self.tree.children:
            self.recursive_search(self.tree.children[c], word, c, row, variants, lev_dist)
        return variants
    
    def recursive_search(self, node, word, c, row, variants, lev_dist):
        if len(variants) > 4:
            return
        columns = len(word) + 1
        current_row = [row[0] + 1]
        for col in range(1, columns):
            replace_dist = row[col - 1] + int(word[col - 1] != c)
            delete_dist = row[col] + 1
            insert_dist = current_row[col - 1] + 1
            current_row.append(min(replace_dist, insert_dist, delete_dist))
            
        if node.word and current_row[-1] <= lev_dist:
            variants.append([node.word, current_row[-1]])
            
        if lev_dist >= min(current_row):
            for c in node.children:
                self.recursive_search(node.children[c], word, c, current_row, variants, lev_dist)
                
    def get_lev_proba(self, word, max_lev = 1):
        correction = self.search(word, max_lev)
        for i in range(len(correction)):
            correction[i][1] = 1.5  ** (-correction[i][1])
        return correction

