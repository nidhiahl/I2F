import numpy as np
from scipy import stats
import random
from skmultiflow.drift_detection.base_drift_detector import BaseDriftDetector
import pandas as pd


#Reference: https://github.com/Elmecio/IForestASD_based_methods_in_scikit_Multiflow/blob/eee23b77dc52de704f559cdfbce06b1b96bd2854/source/ndkswin.py#L16


class NDKSWIN(BaseDriftDetector):
    r""" Kolmogorov-Smirnov Windowing method for concept drift detection.
    Parameters
    ----------
    alpha: float (default=0.005)
        Probability for the test statistic of the Kolmogorov-Smirnov-Test
        The alpha parameter is very sensitive, therefore should be set
        below 0.01.
    window_size: float (default=100)
        Size of the sliding window
    stat_size: float (default=30)
        Size of the statistic window
    ---data: numpy.ndarray of shape (n_samples, 1) (default=None,optional)
        Already collected data to avoid cold start.---
    data: numpy.ndarray of shape (n_samples, n_attributes) (default=None,optional)
        Already collected data to avoid cold start.
    n_dimensions = the number of random dimensions to consider when computing
        stats and detecting the drift

    Notes
    -----
    KSWIN (Kolmogorov-Smirnov Windowing) [1]_ is a concept change detection method based
    on the Kolmogorov-Smirnov (KS) statistical test. KS-test is a statistical test with
    no assumption of underlying data distribution. KSWIN can monitor data or performance
    distributions. ----Note that the detector accepts one dimensional input as array.---
    Note that this version is free of dimensional number input as array
    KSWIN maintains a sliding window :math:`\Psi` of fixed size :math:`n` (window_size). The
    last :math:`r` (stat_size) samples of :math:`\Psi` are assumed to represent the last
    concept considered as :math:`R`. From the first :math:`n-r` samples of :math:`\Psi`,
    :math:`r` samples are uniformly drawn, representing an approximated last concept :math:`W`.
    The KS-test is performed on the windows :math:`R` and :math:`W` of the same size. KS
    -test compares the distance of the empirical cumulative data distribution :math:`dist(R,W)`.
    A concept drift is detected by KSWIN if:
    * :math:`dist(R,W) > \sqrt{-\frac{ln\alpha}{r}}`
    -> The difference in empirical data distributions between the windows :math:`R` and :math:`W`
    is too large as that R and W come from the same distribution.
    References
    ----------
    .. [1] Christoph Raab, Moritz Heusinger, Frank-Michael Schleif, Reactive
       Soft Prototype Computing for Concept Drift Streams, Neurocomputing, 2020,
    """

    def __init__(self, alpha=0.005, window_size=100, stat_size=30, data=None,
                 n_dimensions: int = 2, n_tested_samples=0.01,
                 fixed_checked_dimension=False, fixed_checked_sample=False):
        super().__init__()
        self.window_size = window_size
        self.n_tested_samples = n_tested_samples
        self.fixed_checked_dimension = fixed_checked_dimension
        self.fixed_checked_sample = fixed_checked_sample
        self.n_dimensions = n_dimensions
        self.stat_size = stat_size
        self.alpha = alpha
        self.change_detected = False
        self.p_value = 0
        self.n = 0
        if self.alpha < 0 or self.alpha > 1:
            raise ValueError("Alpha must be between 0 and 1")

        if self.window_size < 0:
            raise ValueError("window_size must be greater than 0")

        if self.window_size < self.stat_size:
            raise ValueError("stat_size must be smaller than window_size")

        self.window = data

        if self.n_dimensions <= 0 or (self.window is not None and self.n_dimensions > data.shape[1]):
            print(
                "Warning: n_dimensions must be between 1 and <= input_value.shape[1]. We will consider all dimensions to compute the drift detection.")
            # raise ValueError("n_dimensions must be between 1 and <= data.shape[1]")
            self.n_dimensions = self.window.shape[1]

        if self.n_tested_samples <= 0.0 or self.n_tested_samples > 1.0:
            raise ValueError("n_tested_samples must be between > 0 and <= 1")
        else:
            self.n_samples_to_test = int(self.window_size * self.n_tested_samples)

    def add_element(self, input_value):
        """ Add element to sliding window
        Adds an element on top of the sliding window and removes
        the oldest one from the window. Afterwards, the KS-test
        is performed.
        Parameters
        ----------
        input_value: ndarray
            New data sample the sliding window should add.
        """

        self.change_detected = False

        if self.fixed_checked_dimension:
            sample_dimensions = list(range(self.n_dimensions))
        else:
            if self.n_dimensions > input_value.shape[1]:
                print(
                    "n_dimensions must be between 1 and <= input_value.shape[1]. We will consider the first dimension only to compute the drift detection.")
                sample_dimensions = [0]
            else:
                sample_dimensions = random.sample(list(range(input_value.shape[1])), self.n_dimensions) #randomly sampling required n-dimensions from data instance dimensions


        if self.fixed_checked_sample:
            sample_test_data = input_value[list(range(self.n_samples_to_test))]
        else:
            if self.n_samples_to_test > input_value.shape[0]:
                sample_test_data = input_value
            else:
                
                sample_test_data = input_value[random.sample(list(range(input_value.shape[0])), self.n_samples_to_test)]

      
        for value in sample_test_data:

            if self.change_detected == False:
                self.n += 1
                currentLength = self.window.shape[0]
                if currentLength >= self.window_size:
                    self.window = np.delete(self.window, 0, 0) #delete first element from row axis from window

                    for i in sample_dimensions:
                        rnd_window = np.random.choice(np.array(pd.DataFrame(self.window)[i])[:-self.stat_size],
                                                      self.stat_size)

                        # print("rnd_window = ")
                        # print(rnd_window)
                        # print("np.array(pd.DataFrame(self.window)[i])[:-self.stat_size] = ")
                        # print(np.array(pd.DataFrame(self.window)[i])[:-self.stat_size])
                        # print("np.array(pd.DataFrame(self.window)[i]) = ")
                        # print(np.array(pd.DataFrame(self.window)[i]))
                        # print("np.array(pd.DataFrame(self.window)[i])[-self.stat_size:] = ")
                        # print(np.array(pd.DataFrame(self.window)[i])[-self.stat_size:])

                        # (st, self.p_value) = stats.ks_2samp(rnd_window,
                        #                                   self.window[:,i][-self.stat_size:], mode="exact")
                        (st, self.p_value) = stats.ks_2samp(rnd_window,
                                                            np.array(pd.DataFrame(self.window)[i])[-self.stat_size:],
                                                            mode="asymp") # exact
                        # print("self.p_value = ")
                        # print(self.p_value)
                        # print("st = ")
                        # print(st)

                        if self.p_value <= self.alpha and st > 0.1:
                            self.change_detected = True
                            self.window = self.window[-self.stat_size:]
                            # print("Change_detected in dimension "+str(i)+" on data "+str(value))
                            break
                        else:
                            self.change_detected = False
                            # print("self.change_detected = False")
                else:  # Not enough samples in sliding window for a valid test
                    # raise ValueError("Not enough samples in sliding window for a valid test")
                    # print("Not enough samples in sliding window for a valid test")
                    self.change_detected = False

                self.window = np.concatenate([self.window, [value]])
            else:
                # print("break execution")
                break

    def detected_change(self):
        """ Get detected change
        Returns
        -------
        bool
            Whether or not a drift occurred
        """
        return self.change_detected

    def reset(self):
        """ reset
        Resets the change detector parameters.
        """
        self.p_value = 0
        self.window = np.array([])
        self.change_detected = False