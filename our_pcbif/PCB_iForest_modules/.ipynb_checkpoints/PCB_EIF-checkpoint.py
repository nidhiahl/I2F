
import numpy as np
import random as rn
import time
from .NDKSWIN import NDKSWIN as ndk
import math


class PCB_EIF_Window():

    """ count-based Sliding windowing.
        """

    def __init__(self, dimension, window_size: int = 256, num_trees: int = 100, anomaly_threshold: float = 0.5,
                 extension_level: int = 0, initial_data_size: int = 256, update_data_size:int = 256,
                 # Parameters for NDKSWIN
                 alpha = 0.01, n_dimensions = 1, n_tested_samples = 0.1, stat_size = 30,
                 fixed_checked_dimension = False, fixed_checked_sample = False):

        self.initial_data= []
        self.initial_data_size = initial_data_size
        self.update_data_size = update_data_size
        self.window = []
        self.window_size = window_size
        self.name = "Performance-counter based Isolation Forest for streaming data"
        self.n_samples_count = 0
        self.samples_count_after_training = 0
        self.init_flag = False
        self.F = None
        self.num_trees = num_trees
        self.threshold = anomaly_threshold
        self.extension = extension_level
        self.final_score = 0
        self.prediction = 0
        self.prec_window = None

        self.dimension = dimension # sample's dimension

        self.alpha = alpha
        self.n_dimensions = n_dimensions
        self.n_tested_samples = n_tested_samples
        self.stat_size = stat_size
        self.fixed_checked_dimension = fixed_checked_dimension
        self.fixed_checked_sample = fixed_checked_sample
        self.ndkswindow = None
        self.update_counter = 0
        self.ascore = []
        self.prev_update_n_samples_count = 0
    
#run_train_classifier definition when D0 is parsed fully and remaining data is parsed only using windows
    
    def run_train_classifier(self, input:np.ndarray):

        """create and update the classifier model

        @param  ndarray
        @returns prediction """

        if len(input) == 0:
            raise Exception("No stream detected!")
        else:
            data_point = np.reshape(input,(1,len(input)))
            num_dimensions = data_point.shape[1]
            self.samples_count_after_training += 1
            if not self.init_flag:
                self.initial_data.append(input)
                
                #print(self.samples_count_after_training, len(self.initial_data), self.initial_data_size) 
                
                if len(self.initial_data) == self.initial_data_size:
                    self.F = EiForest(self.initial_data, ntrees=self.num_trees, sample_size=256, threshold=self.threshold, ExtensionLevel=self.extension)
                    self.F.ensemble_fit(self.initial_data)  # EiForest model trained
                    self.init_flag = True
                    for point in self.initial_data:
                    	overall_ascore, individual_ascore = self.F.compute_paths(input) # self.window[-1]
                    	self.ascore.append(overall_ascore)
                
                    #print(self.init_flag)
            else:
                
                self.n_samples_count += 1
                self.window.append(input)
                #print(self.samples_count_after_training, self.init_flag, self.n_samples_count, len(self.window))
                overall_anomaly_score, individual_anomaly_score = self.F.compute_paths(input) # self.window[-1]
                self.F.compare_scores(overall_anomaly_score, individual_anomaly_score)
                self.prediction = self.F.predict_function(overall_anomaly_score)
                
                if (len(self.window) == self.window_size):
                    self.ndkswindow = ndk(alpha=self.alpha, data=np.asarray(self.window), n_dimensions=self.n_dimensions, window_size=self.window_size, stat_size=self.stat_size, n_tested_samples=self.n_tested_samples, fixed_checked_dimension=self.fixed_checked_dimension, fixed_checked_sample=self.fixed_checked_sample)
                    #print("ndkswin initialized")
                    
                if (len(self.window) > self.window_size):
                    self.window.pop(0)
                    
                #print("window_growing",len(self.window),self.window_size,self.n_samples_count % self.window_size)
                    
                if (self.n_samples_count % self.window_size == 0 and self.n_samples_count !=0):
                    self.ndkswindow.add_element(np.asarray(self.window))
                    if self.ndkswindow.detected_change():
                        self.update_counter += 1
                        self.prev_update_n_samples_count = self.n_samples_count
                        self.F.update_model(window=self.window)
                            for point in self.window:
                                overall_ascore, individual_ascore = self.F.compute_paths(input) # self.window[-1]
                                self.ascore.append(overall_ascore)
                else if self.n_samples_count == self.update_data_size:
                    self.ndkswindow.add_element(np.asarray(self.window))
                    if self.ndkswindow.detected_change():
                        self.update_counter += 1

                        self.F.update_model(window=self.window)
                            for point in self.window[self.prev_update_n_samples_count+1:]:
                                overall_ascore, individual_ascore = self.F.compute_paths(input) # self.window[-1]
                                self.ascore.append(overall_ascore)
                    
                        #print("model update")
    
    def give_prediction(self):
        return self.prediction
    def give_ascore(self,id:int):
        return self.ascore[id]
    def predict(self, input:np.ndarray):
        overall_anomaly_score, individual_anomaly_score = self.F.compute_paths(input) # self.window[-1]
        self.F.compare_scores(overall_anomaly_score, individual_anomaly_score)
        pr = self.F.predict_function(overall_anomaly_score)
        return pr

''' #run_train_classifier definition when complete data is parsed only using windows
    def run_train_classifier(self, input:np.ndarray):

        """create and update the classifier model

        @param  ndarray
        @returns prediction """
           
        if len(input) == 0:
            raise Exception("No stream detected!")
        else:
            data_point = np.reshape(input,(1,len(input)))
            num_dimensions = data_point.shape[1]
            self.n_samples_count += 1
            self.window.append(input)
            if self.init_flag is True:
                overall_anomaly_score, individual_anomaly_score = self.F.compute_paths(input) # self.window[-1]
                self.F.compare_scores(overall_anomaly_score, individual_anomaly_score)
                self.prediction = self.F.predict_function(overall_anomaly_score)
                if (len(self.window) > self.window_size):
                    self.window.pop(0)

            if self.n_samples_count % self.window_size == 0 and self.n_samples_count !=0:
                if not self.init_flag:

                    self.F = EiForest(self.window, ntrees=self.num_trees, sample_size=self.window_size,
                                         threshold=self.threshold, ExtensionLevel=self.extension)
                    self.F.ensemble_fit(self.window)  # EiForest model trained
                    self.ndkswindow = ndk(alpha=self.alpha, data=np.asarray(self.window),
                                                      n_dimensions=self.n_dimensions,
                                                      window_size=self.window_size, stat_size=self.stat_size,
                                                      n_tested_samples=self.n_tested_samples,
                                                      fixed_checked_dimension=self.fixed_checked_dimension,
                                                      fixed_checked_sample=self.fixed_checked_sample)
                    self.init_flag = True
                else:
                    self.ndkswindow.add_element(np.asarray(self.window))
                    if self.ndkswindow.detected_change():
                        self.update_counter += 1
                        self.F.update_model(window=self.window)
'''



#original source code adopted and modified for our project https://github.com/sahandha/eif

def c_factor(n):
    return 2.0 * (np.log(n - 1) + 0.5772156649) - (2.0 * (n - 1.) / (n * 1.0))


class EiForest(object):
    def __init__(self, X, ntrees, sample_size, limit=None, threshold:float = 0.5, ExtensionLevel=0):
        self.name= "Extended Isolation Forest"
        self.ntrees = ntrees
        self.X = X
        self.nobjs = len(X)
        self.sample = sample_size
        self.Trees = []

        self.perfCounter = [0] * self.ntrees
        self.individual_score = []
        self.window = None

        self.limit = limit
        self.threshold = threshold
        self.exlevel = ExtensionLevel
        self.CheckExtensionLevel()  # Extension Level check. See def for explanation.


    def ensemble_fit(self, X):   #training the ensemble
        X = np.asarray(X)
        if self.limit is None:
            self.limit = int(np.ceil(np.log2(
                self.sample)))  # Set limit to the default as specified by the paper (average depth of unsuccesful search through a binary tree).
        self.c = c_factor(self.sample)
        for i in range(self.ntrees):  # This loop builds an ensemble of iTrees (the forest).
            ix = rn.sample(range(self.nobjs), self.sample)
            X_p = X[ix]
            tree = iTree(X_p, 0, self.limit, exlevel=self.exlevel)
            self.Trees.append(tree)

    def CheckExtensionLevel(self):
        dim = len(self.X[0])
        if self.exlevel < 0:
            raise Exception("Extension level has to be an integer between 0 and " + str(dim - 1) + ".")
        if self.exlevel > dim - 1:
            raise Exception(
                "Your data has " + str(dim) + " dimensions. Extension level can't be higher than " + str(dim - 1) + ".")

    def compute_paths(self, X_in=None):
        individual_score = np.zeros(self.ntrees)
        h_temp = 0
        for j in range(self.ntrees):
            single_tree_each_instance_pathlength = PathFactor(X_in, self.Trees[j]).path * 1.0
            h_temp += single_tree_each_instance_pathlength  # Compute path length for each point
            individual_score[j] = 2.0 ** (-single_tree_each_instance_pathlength / self.c)
        Eh = h_temp / self.ntrees  # Average of path length travelled by the point in all trees.
        S = 2.0 ** (-Eh / self.c)  # Anomaly Score
        return S, individual_score


    def partial_fit(self, window:list): #window : list of ndarray
        '''
        @param window: list of ndarray
        @return: None
        '''
        window = np.asarray(window)
        temp_tree = iTree(window, 0, self.limit, exlevel=self.exlevel)
        return temp_tree

    def compare_scores(self, S:int, individual_score:np.ndarray):
        '''
        compare overall ensemble score for an instance with individual score for
        an instance by each itree and set the PC accordingly

        @param S: overall ensemble score for an instance
        @param individual_score: individual score for an instance by each itree
        @return: None
        '''

        for j in range(self.ntrees):
            if S > self.threshold:
                if self.threshold > individual_score[j]:
                    self.perfCounter[j] -= 1
                else:
                    self.perfCounter[j] += 1
            else:
                if self.threshold < individual_score[j]:
                    self.perfCounter[j] -= 1
                else:
                    self.perfCounter[j] += 1



    def update_model(self,window:list):
        '''
        update the model by replacing the weak itrees in the ensemble with new trees built on
        the current instances in the window.

        @param window: list of ndarray
        @return: None
        '''

        for j in range(self.ntrees):
            if self.perfCounter[j] < 0 :
                self.Trees.pop(j)     #replace weak-performing itrees with new trees built with window
                new_tree = self.partial_fit(window)
                self.Trees.insert(j,new_tree)
        self.perfCounter = [0] * self.ntrees


    def predict_function(self, S):
        '''
        predicts if the instance is anomaly or not.

        @param S: overall ensemble score for an instance
        @return: 1 or 0
        '''
        if (S > self.threshold):
            return 1   #anomalous point is represented as 1
        else:
            return 0  #non-anomalous point



class Node(object):
    def __init__(self, X, n, p, e, left, right, node_type=''):
        self.e = e
        self.size = len(X)
        self.X = X
        self.n = n
        self.p = p
        self.left = left
        self.right = right
        self.ntype = node_type


class iTree(object):
    def __init__(self, X, e, l, exlevel=0):
        self.exlevel = exlevel
        self.e = e   #current depth of tree
        self.X = X  # save data for now. Not really necessary.
        self.size = len(X)
        self.dim = self.X.shape[1]
        self.Q = np.arange(np.shape(X)[1], dtype='int')  # n dimensions
        self.l = l
        self.p = None  # Intercept for the hyperplane for splitting data at a given node.
        self.n = None  # Normal vector for the hyperplane for splitting data at a given node.
        self.exnodes = 0

        self.root = self.make_tree(X, e, l)  # At each node create a new tree, starting with root node.

    def make_tree(self, X, e, l):
        self.e = e
        if e >= l or len(X) <= 1:  # A point is isolated in training data, or the depth limit has been reached.
            left = None
            right = None
            self.exnodes += 1
            return Node(X, self.n, self.p, e, left, right, node_type='exNode')
        else:  # Building the tree continues. All these nodes are internal.
            mins = X.min(axis=0)
            maxs = X.max(axis=0)

            idxs = np.random.choice(range(self.dim), self.dim - self.exlevel - 1,
                                    replace=False)  # Pick the indices for which the normal vector elements should be set to zero acccording to the extension level.

            self.n = np.random.normal(0, 1,
                                      self.dim)  # A random normal vector picked form a uniform n-sphere. Note that in order to pick uniformly from n-sphere, we need to pick a random normal for each component of this vector.
            self.n[idxs] = 0
            self.p = np.random.uniform(mins,
                                       maxs)  # Picking a random intercept point for the hyperplane splitting data.
            w = (X - self.p).dot(
                self.n) < 0  # Criteria that determines if a data point should go to the left or right child node.


            return Node(X, self.n, self.p, e, \
                        left=self.make_tree(X[w], e + 1, l), \
                        right=self.make_tree(X[~w], e + 1, l), \
                        node_type='inNode')


class PathFactor(object):
    def __init__(self, x, itree):
        self.path_list = []
        self.x = x
        self.e = 0
        self.path = self.find_path(itree.root)

    def find_path(self, T):
        if T.ntype == 'exNode':
            if T.size <= 1:
                return self.e
            else:
                self.e = self.e + c_factor(T.size)
                return self.e
        else:
            p = T.p  # Intercept for the hyperplane for splitting data at a given node.
            n = T.n  # Normal vector for the hyperplane for splitting data at a given node.

            self.e += 1

            if (self.x - p).dot(n) < 0:
                self.path_list.append('L')
                return self.find_path(T.left)
            else:
                self.path_list.append('R')
                return self.find_path(T.right)
