import pathlib
import os
import sys
import unittest
import numpy as np
from .tools import debug_on
from numpy.polynomial.legendre import Legendre
import matplotlib.pyplot as plt
import time
try:
    from pygsplines import BasisLegendre
    from pygsplines import PyInterpolator as Interpolator
except ImportError:
    MOD_PATH = pathlib.Path(__file__).parent.absolute()
    MOD_PATH_PYGSPLINES = pathlib.Path(MOD_PATH, '..', 'build')
    sys.path.append(str(MOD_PATH_PYGSPLINES))
    from pygsplines import BasisLegendre
    from pygsplines import PyInterpolator as Interpolator


def show_piecewisefunction(_q, _up_to_deriv=3, _dt=0.1, _title=''):
    dim = _q.get_codom_dim()
    fig, ax = plt.subplots(_up_to_deriv + 1, dim)
    if dim == 1:
        ax = np.array([[ax[i]] for i in range(_up_to_deriv + 1)])
    if _title:
        fig.suptitle(_title)
    t = np.arange(0.0, _q.get_exec_time(), _dt)

    for i in range(0, _up_to_deriv + 1):
        q = _q.deriv(i)
        qt = q(t)
        for j in range(0, dim):
            ax[i, j].plot(t, qt[:, j])
            ax[i, j].grid()
            if i == 0:
                ax[i, j].set_title('coordinate {:d}'.format(j + 1), fontsize=8)

            if hasattr(_q, 'get_domain_breakpoints'):
                for ti in _q.get_domain_breakpoints():
                    ax[i, j].axvline(ti, alpha=0.1, color='red')

    plt.subplots_adjust(
        left=0.025,
        bottom=0.05,
        right=0.975,
        top=0.95,
        wspace=0.25,
        hspace=0.15)
    plt.show()


class MyTest(unittest.TestCase):
    @debug_on()
    def __init__(self, *args, **kwargs):
        super(MyTest, self).__init__(*args, **kwargs)
        np.random.seed()

    @debug_on()
    def test(self):
        basis = BasisLegendre(2*np.random.randint(1, 5))
        dim = np.random.randint(1, 10)
        intervals = np.random.randint(1, 6)
        waypoints = np.random.rand(intervals+1, dim)
        interval_lengths = 1.0 + np.random.rand(intervals) * 2.0
        inter = Interpolator(dim, intervals, basis)
        res = inter.interpolate(interval_lengths, waypoints)
#        inter.print_interpolating_matrix()
#        inter.print_interpolating_vector()

        show_piecewisefunction(res, 5, 0.001)

    @debug_on()
    def test_interpolation_time(self):
        basis = BasisLegendre(6)
        dim = 7
        intervals = 10
        waypoints = np.random.rand(intervals+1, dim)
        intervals_length = np.array(intervals*[1])

        iters = 100
        tmean = 0
        for _ in range(iters):
            time_0 = time.time()
            inter = Interpolator(dim, intervals, basis)
            _ = inter.interpolate(intervals_length, waypoints)
            time_1 = time.time()
            tmean += (time_1 - time_0)

        print('mean time = ', tmean/iters)

    @debug_on()
    def test_derivative_y(self):
        ''' Compare the numerical derivate of y w.r.t tau with the nominal one
        '''
        for i in range(40):
            print('--- begin --')
            b_dim = 2*np.random.randint(1, 10)
            basis = BasisLegendre(b_dim)
            np.random.seed()
            dim = np.random.randint(1, 6)
            intervals = np.random.randint(2, 10)
            waypoints = (np.random.rand(intervals + 1, dim) - 0.5) * 2 * np.pi
            interval_lengths = 1.0 + np.random.rand(intervals) * 2.0
            print('interplator constrcutor codo_dim, N, bdim',
                  dim, intervals, b_dim)
            inter = Interpolator(dim, intervals, basis)
            y_nom = inter.solve_interpolation(
                interval_lengths, waypoints)

            dtau = 1.0e-5
            err = 0.0
            errp = 0.0
            for iinter in range(0, intervals):
                dydtauNom = inter.get_coeff_derivative_wrt_tau(
                    y_nom, interval_lengths, iinter)

                tauv_aux = interval_lengths.copy()
                tauv_aux[iinter] += -2 * dtau
                y0 = inter.solve_interpolation(
                    tauv_aux, waypoints).copy() * (1.0 / 12.0)
                tauv_aux[iinter] += dtau
                y1 = inter.solve_interpolation(
                    tauv_aux, waypoints).copy() * (-2.0 / 3.0)
                tauv_aux[iinter] += 2 * dtau
                y2 = inter.solve_interpolation(
                    tauv_aux, waypoints).copy() * (2.0 / 3.0)
                tauv_aux[iinter] += dtau
                y3 = inter.solve_interpolation(
                    tauv_aux, waypoints).copy() * (-1.0 / 12.0)
                dydtauiTest = (y0 + y1 + y2 + y3) / dtau
                ev = np.abs(dydtauiTest - dydtauNom)
                e = np.max(ev)
                eidx = np.argmax(ev)

                if(abs(dydtauiTest[eidx]) > 1.0e-5):
                    ep = e / abs(dydtauiTest[eidx])
                else:
                    ep = e

                if e > err:
                    err = e
                if ep > errp:
                    errp = ep

                assert ep < 5.0e-2, '''
                error on dydtau = {:10.7e}
                relative error  = {:10.7e}
                value of dydtau test = {:10.7e}
                value of dydtau nom = {:10.7e}
                '''.format(e, ep, dydtauiTest[eidx], dydtauNom[eidx])


if __name__ == '__main__':
    unittest.main()
