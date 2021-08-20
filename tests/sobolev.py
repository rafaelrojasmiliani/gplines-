"""
    Test the cost function from the problem 1010
"""
import numpy as np
from scipy.special import roots_legendre
import unittest
import sys
import pathlib

from itertools import tee

try:
    from gsplines import BasisLegendre
    from gsplines import PyInterpolator as Interpolator
    from gsplines import SobolevNorm
except ImportError:
    MOD_PATH = pathlib.Path(__file__).parent.absolute()
    MOD_PATH_PYGSPLINES = pathlib.Path(MOD_PATH, '..', 'build')
    sys.path.append(str(MOD_PATH_PYGSPLINES))
    from gsplines import BasisLegendre
    from gsplines import SobolevNorm
    from gsplines import PyInterpolator as Interpolator


def pairwise(iterable):
    '''s -> (s0,s1), (s1,s2), (s2, s3), ...'''
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


class cMyTest(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(cMyTest, self).__init__(*args, **kwargs)
        print('a')
        np.random.seed()
        self.N_ = N = 6  # np.random.randint(2, 10)
        self.dim_ = dim = 2  # np.random.randint(1, 8)
        self.wp_ = wp = np.random.rand(N + 1, dim)
        b_dim = 6
        basis = BasisLegendre(b_dim)
        self.basis_ = basis
        self.cost_ = SobolevNorm(wp, basis, [(3, 1.0)])
        self.inter_ = Interpolator(dim, N, basis)
        print('b')

    def test_value(self):
        print('testing value')
        wp = self.wp_
        N = self.N_
        tauv = np.array([1.0]*N)

        cost = self.cost_
        q = self.inter_.interpolate(tauv, wp)
        print('------')

        qd = q.deriv(1)
        print('------')
        qdd = q.deriv(2)
        print('------')
        qddd = q.deriv(3)

        def qd3norm(t):
            return np.einsum('ij,ij->i', qdd(t), qdd(t))

        def runningcost(t):
            return qd3norm(t)

        print('b')
        Inom = cost(tauv)
        err = 1.e100
        badtrentCounter = 0
        for Ngl in [12, 200]:
            lr, lw = roots_legendre(Ngl)
            time_partition = q.get_domain_breakpoints()
            Itest_1 = 0.0
            for t0, tf in pairwise(time_partition):
                Itest_1 += sum([w*runningcost(np.array([s]))[0]*(tf-t0)/2.0
                                for w, s in zip(lw, (lr+1.0)/2.0*(tf-t0)+t0)])
            if abs(Itest_1 - Inom) > err:
                badtrentCounter += 1
            else:
                badtrentCounter = 0
            assert badtrentCounter < 3
            err = abs(Itest_1 - Inom)
            if err > 1.0e-4:
                print('Error w.r.t quadrature = {:.3f}'.format(err))
                print('Nomial cost  = {:14.7e}, Quadrature = {:14.7e}'.format(
                    Inom, Itest_1))
            if err < 1.0e-4:
                break

#    def test_gradient(self):
#        print('a')
#        N = self.N_
#
#        cost = self.cost_
#        nom_grad = np.zeros((N, ))
#        test_grad = np.zeros((N, ))
#        dt = 1.0e-6
#        print('a')
#        for _ in range(3):
#            tauv = 1+np.random.rand(N)*2
#            cost.deriv_wrt_interval_len(tauv, nom_grad)
#            for i in range(N):
#                tauv_aux = tauv.copy()
#                tauv_aux[i] -= dt
#                cost0 = cost(tauv_aux)
#                tauv_aux[i] += 2*dt
#                cost1 = cost(tauv_aux)
#                test_grad[i] = (cost1 - cost0)/(2*dt)
#            err = np.linalg.norm(test_grad - nom_grad)
#            print('test grad err', err)
#            print(test_grad - nom_grad)
#            print(nom_grad)
#            print(test_grad)
#


def main():
    unittest.main()


if __name__ == '__main__':
    main()
