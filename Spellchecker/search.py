import itertools
from collections import defaultdict
from classifier import Features, Classifier

class Spellchecker:
    
    def __init__(self, language_model, clear_language_model, bor, keyboard, classifier):
        self.language_model = language_model
        self.clear_language_model = clear_language_model
        self.clf = classifier
        self.bor = bor
        self.keyboard = keyboard
        self.features = Features(self.language_model)
        self.MAX_FIXES = 500

    def fix_split(self, words):
        splits = []
        query = []

        for i in range(len(words)):
            word = words[i]
    #             if word in fix: ###CLERAN_LANGUAGE_MODEL
    #                 continue
            for j in range(1, len(word)):
                split = words[0:i]
                split.append(word[0:j])
                split.append(word[j:])
                split.extend(words[i + 1:])
                splits.append(split)
        maximum = -1e20
        best_variant = None
        for variant in splits:
            weight = self.language_model.get_query_weight(variant)
#             print(variant, weight)
            if weight > maximum:
                maximum = weight
                best_variant = variant
#                 print(variant, weight)
        if best_variant is not None:
            str_best_var = ' '.join([str(x) for x in best_variant])
            return maximum, str_best_var
        return 0, ' '.join([str(x) for x in words])

    
    def fix_join(self, words):
        if len(words) < 2:
#             print('aaaa', self.classifier.simple_word_clf(words[0]))
            return self.language_model.get_query_weight(words[0]), str(words[0])
        joins = []
        for i in range(len(words) - 1):
            join = words[0:i]
            join.append(words[i] + words[i + 1])
            join.extend(words[i + 2:])
            joins.append(join)
#         print(joins)
        maximum = -1e20
        best_variant = None
        for variant in joins:
            weight = self.language_model.get_query_weight(variant)
#             print('weight jf', weight)
            if weight > maximum:
                maximum = weight
                best_variant = variant
        original_weight = self.language_model.get_query_weight(words)
        if maximum < original_weight:
            return original_weight, ' '.join([ x for x in words])
        str_best_var = ' '.join([str(x) for x in best_variant])
#         print()
        return maximum, str_best_var

    def fix_keyboard(self, query):
        result = []
        for word in query:
            checked = self.keyboard.change_keyboard(word)
            if checked:
                result.append(self.keyboard.change_keyboard(word))
            else:
                result.append(word)
        return ' '.join([str(x) for x in result])

    
    def fix_orpho(self, query):
        iterations = 0
        variants = []
        for word in query:
            if word in self.clear_language_model.language_dict:
                variants.append([word])
                continue
            variants.append([])
            variant = self.bor.get_lev_proba(word, 1)
#             if not variant:
#                 variant = bor_tree.get_lev_proba(word, 2)   
#             print(variant)
            for word, p in variant:
                variants[-1].append(word)
            if len(variants[-1]) == 0:
                variants[-1].append(word)
#             else:
#                 return None, 0
#        print(len(variants))
        variants = itertools.product(*variants)
        combinations = defaultdict()
#        print(len(variants))
#        print(variants)
        for var in variants:
            if iterations > self.MAX_FIXES:
                break
#            print(list(var))
            combinations[var] = self.language_model.get_query_weight(var)
            x = combinations[var]
            iterations +=1
#            print(x)
#        print(len(combinations))
        if(len(combinations) > 1 and all(x == combinations[var] for var in combinations.keys())):
#            print("пересчитываем веса")
            for var in combinations.keys():
#                print(var)
#                print(type(var))
                combinations[var] = self.language_model.get_unigram_weight(var)
#         print(combinations)
#         print(max(combinations, key = lambda x: combinations[x]))
        result = max(combinations, key = lambda x: combinations[x])
        return ' '.join([str(x) for x in result]), combinations[result]

    def predict(self, query, words):
        x = [self.features.get_features(query, words)]
        return self.clf.model.predict_proba(x)[0][1]
    
    
    def query_language_model_clf(self, query):
        words = query.split()
        count = 0
        for word in words:
            if word in self.language_model.language_dict:
                count += 1
        return count
            
    
    def fix_query(self, check_query):
        if check_query == "":
            return ""
        if len(check_query.split()) > 25:
            return check_query
        flag = True
        iteration =0 
        y_pred = {}
        weight = {}
        queries = set([check_query])
        while iteration < 3:           
            
            new_query = set()
            for query in queries:
                
#                 print(self.predict(query, query.split()))    

                
                words = query.lower().split()
#                 print(iteration, query)
                if flag:
                    keyboard_fix = self.fix_keyboard(words)
                    new_query.add(keyboard_fix)
#                    print(keyboard_fix)
                    flag = False
                    continue
                
                word_fix, proba_wf = self.fix_orpho(words)
                if word_fix is not None:
#                     print('wf', word_fix)
                    if word_fix not in y_pred.keys():
                        y_pred[word_fix] = self.predict(word_fix, word_fix.split())
                        weight[word_fix] = self.clear_language_model.get_query_weight(word_fix.split())
                        new_query.add(word_fix)
                        
#                     print(new_query)

                split_fix = self.fix_split(words)[1]
                if split_fix not in y_pred.keys():
#                     weight = self.classifier.bigrams_classifier(split_fix.split())
#                     if weight > -120:
                    y = self.predict(split_fix, split_fix.split())
                    z = self.clear_language_model.get_query_weight(split_fix.split())
                    if y > self.predict(query, query.split()) and z > self.clear_language_model.get_query_weight(query.split()):
#                         print('sf', split_fix)
    
                        y_pred[split_fix] = y
                        weight[split_fix] = z
                        new_query.add(split_fix)
                        
                join_fix = self.fix_join(words)[1]
#                 print('jf', join_fix)
                if join_fix not in y_pred.keys():
                    y = self.predict(join_fix, join_fix.split())
                    z = self.clear_language_model.get_query_weight(join_fix.split())
                    if y > self.predict(query, query.split()) and z > self.clear_language_model.get_query_weight(query.split()):
#                         print('jf', join_fix)

                        y_pred[join_fix] = self.predict(join_fix, join_fix.split())
                        weight[join_fix] = self.clear_language_model.get_query_weight(join_fix.split())
                        new_query.add(join_fix)
#                     if weight > -120:
#                         prev[join_fix] = weight
#                 print("", y_pred)

                if max(y_pred.items(), key = lambda x:x[1])[1] > 0.99:
                    return max(y_pred.items(), key = lambda x:x[1])[0] 
            queries = new_query
#             for variant in new_query:
#                 y = self.predict(variant, variant.split())
#                 print(variant, y)
            iteration += 1
#             print(y_pred)
        pred_query, max_pred = max(y_pred.items(), key = lambda x:x[1])
        weight_query, max_weight = max(weight.items(), key = lambda x:x[1])
#         print("sdf", y_pred)
#         print(weight)
        if max_pred > 0.98:
            return pred_query
        else:
            return weight_query
