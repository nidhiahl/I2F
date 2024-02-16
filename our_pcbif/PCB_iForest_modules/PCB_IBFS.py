import numpy as np
import random as rn
import time
from .NDKSWIN import NDKSWIN as ndk
import math

class IncrementalAverage():
    # according to Welford's formula
    # d = dimensions
    # n = counter
    # average = array of d features for moving average
    def __init__(self, d):
        self.n = 0
        self.d = d
        self.average = np.zeros(self.d)

    def __call__(self, new_value):
        self.n += 1
        for i in range(self.d-1):
            self.average[i] = (self.average[i] * (self.n - 1) + new_value[i]) / self.n

    def __float__(self):
        return self.average

    def __repr__(self):
       return "average: " + str(self.average)




class PCB_IBFS_Window():

    """ count-based Sliding windowing.
        """

    def __init__(self, dimension, window_size: int = 256, num_trees: int = 100, anomaly_threshold: float = 0.5, extension_level=None,
                 # Parameters for NDKSWIN
                 alpha = 0.01, n_dimensions = 1, n_tested_samples = 0.1, stat_size = 30,
                 fixed_checked_dimension = False, fixed_checked_sample = False):

        self.window = []
        self.window_size = window_size
        self.name = "Performance-counter based Isolation Forest for streaming data"
        self.n_samples_count = 0
        self.samples_count_after_training = 0
        self.init_flag = False
        self.F = None
        self.num_trees = num_trees
        self.threshold = anomaly_threshold
        self.final_score = 0
        self.prediction = 0
        self.extension = extension_level # Not used in classic iForest

        self.dimension = dimension # sample's dimension

        self.feature_scores = IncrementalAverage(dimension)

        self.alpha = alpha
        self.n_dimensions = n_dimensions # for NDKSWIN!!! not the sample's dimension
        self.n_tested_samples = n_tested_samples
        self.stat_size = stat_size
        self.fixed_checked_dimension = fixed_checked_dimension
        self.fixed_checked_sample = fixed_checked_sample
        self.ndkswindow = None
        self.update_counter = 0

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
                overall_anomaly_score, individual_anomaly_score = self.F.compute_paths(input)  # self.window[-1]
                self.F.compare_scores(overall_anomaly_score, individual_anomaly_score)
                self.prediction = self.F.predict_function(overall_anomaly_score)
                if (len(self.window) > self.window_size):
                    self.window.pop(0)

            if self.n_samples_count % self.window_size == 0 and self.n_samples_count !=0:
                if not self.init_flag:

                    self.F = iForest(self.window, ntrees=self.num_trees, sample_size=self.window_size,
                                         threshold=self.threshold)

                    self.F.ensemble_fit(self.window)  # IForest model trained
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

                        # update feature_scores with incremental averaging
                        self.feature_scores(self.F.feature_scoring())

                        self.F.update_model(window=self.window)

    def give_prediction(self):
        return self.prediction

    def give_feature_scores(self):
        self.feature_scores(self.F.feature_scoring())
        return self.feature_scores.average


#=================================================================================================
"isolated forest functions"
__author__ = 'Matias Carrasco Kind'

import numpy as np
import random as rn
import warnings
import os


def c_factor(n):
    return 2.0 * (np.log(n - 1) + 0.5772156649) - (2.0 * (n - 1.) / (n * 1.0))


class iForest(object):
    def __init__(self, X, ntrees, sample_size, limit=None,threshold:float = 0.5):
        self.ntrees = ntrees
        self.X = X
        self.nobjs = len(X)
        self.sample = sample_size
        self.Trees = []

        self.perfCounter = [0] * self.ntrees
        self.individual_score = []

        self.limit = limit
        self.threshold = threshold

        self.S_forest = []
        self.S_forest_list = list()

    def ensemble_fit(self, X):  # training the ensemble
        X = np.asarray(X)
        if self.limit is None:
            self.limit = int(np.ceil(np.log2(self.sample)))
        self.c = c_factor(self.sample)
        for i in range(self.ntrees):
            ix = rn.sample(range(self.nobjs), self.sample)
            X_p = X[ix]
            tree = iTree(X_p, 0, self.limit, self.sample)
            self.Trees.append(tree)
            # Todo add S_trees (Algorithm 1: line 6)
            S_tree = tree.compute_S_tree()
            self.S_forest_list.append(S_tree)

        # Todo add S_forest (Algorithm 1: line 8-11)
        self.S_forest = np.array(self.S_forest_list)
        self.S_forest = np.sum(self.S_forest, axis=0)/self.ntrees



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
        temp_tree = iTree(window, 0, self.limit, sample_size= len(window))
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

    def feature_scoring(self):
        return self.S_forest


class Node(object):
    def __init__(self, X, q, p, e, left, right, s_theta, n_n, node_type=''):
        self.e = e
        self.size = len(X)
        self.X = X  # to be removed
        self.q = q # split feature
        self.p = p # split value
        self.left = left
        self.right = right
        # Todo add s_theta and n_n for more neater solution
        self.s_theta = s_theta #
        self.n_n = n_n  # length of samples in node, len(X)
        self.ntype = node_type


class iTree(object):
    """
    Unique entries for X
    """

    def __init__(self, X, e, l, sample_size):
        self.e = e  # depth
        self.X = X  # save data for now
        self.size = len(X)  # n objects
        self.Q = np.arange(np.shape(X)[1], dtype='int')  # n dimensions
        self.l = l  # depth limit
        self.p = None
        self.q = None
        self.exnodes = 0

        # Todo not the perfect solution
        self.S_theta = list()
        self.N_n = list()
        self.splitfeaturelist = list()
        self.sample_size = sample_size

        self.root = self.make_tree(X, e, l)



    def make_tree(self, X, e, l):
        self.e = e
        if e >= l or len(X) <= 1:
            left = None
            right = None
            self.exnodes += 1
            # Todo add s_tree (Algorithm 2: line 2-5) -> Moved to dedicated function
            return Node(X, self.q, self.p, e, left, right, None, None, node_type='exNode')
        else:
            self.q = rn.choice(self.Q) # random feature selected
            mini = X[:, self.q].min()
            maxi = X[:, self.q].max()
            if mini == maxi:
                left = None
                right = None
                self.exnodes += 1
                return Node(X, self.q, self.p, e, left, right, None, None, node_type='exNode')
            self.p = rn.uniform(mini, maxi) #selecting random value for splitting
            w = np.where(X[:, self.q] < self.p, True, False)

            # Todo add s_theta (Algorithm 2: line 13)
            n_n = len(X) # length of samples in node, len(X)
            n_l = len(X[w]) #length of samples in left node
            p_l = n_l/n_n   # p_l, proportion of samples split into the left node.
            p_0 = (self.p - mini) / (maxi - mini)    #Min-Max scaling of feature q, feature normalization
            theta = -p_0 * np.log2(p_0) - (1-p_0) * np.log2(1-p_0)  #penality score; higher value of theta(θ), lower penalty
            s = 1+ p_l * np.log2(p_l) + (1-p_l) * np.log2(1-p_l)  #imbalance score s based on entropy function
            s_theta = theta*s   #penalized score
            self.splitfeaturelist.append(self.q)
            self.S_theta.append(s_theta)
            self.N_n.append(n_n)

            return Node(X, self.q, self.p, e, \
                        left=self.make_tree(X[w], e + 1, l), \
                        right=self.make_tree(X[~w], e + 1, l), \
                        s_theta=s_theta, \
                        n_n=n_n, \
                        node_type='inNode')
                        # Todo add S_theta and N_n (Algorithm 2: line 18-19)


    def get_node(self, path):
        node = self.root
        for p in path:
            if p == 'L': node = node.left
            if p == 'R': node = node.right
        return node

    # Todo Algorithm 2: line 2-5
    def compute_S_tree(self):
        S_tree = list()
        for j in range(0, len(self.Q)):
            #print(self.splitfeaturelist)
            indices = np.where(np.array(self.splitfeaturelist)==j)
            s_tree_j = np.sum(np.multiply(np.array(self.S_theta)[indices],np.array(self.N_n)[indices]/self.sample_size))
            S_tree.append(s_tree_j)
        return S_tree


class PathFactor(object):
    def __init__(self, x, itree):
        self.path_list = []
        self.x = x
        self.e = 0
        self.path = self.find_path(itree.root)

    def find_path(self, T):
        if T.ntype == 'exNode':
            if T.size == 1:
                return self.e
            else:
                self.e = self.e + c_factor(T.size)
                return self.e
        else:
            a = T.q
            self.e += 1
            if self.x[a] < T.p:
                self.path_list.append('L')
                return self.find_path(T.left)
            else:
                self.path_list.append('R')
                return self.find_path(T.right)