"""Test the light travel time delay."""
from starry import Star, Planet, System
import numpy as np


def test_light_travel():
    """Run the test."""
    # No delay
    star = Star()
    star.map[1] = 0.40
    star.map[2] = 0.26
    planet = Planet(L=1e-3, r=0.1, porb=10, prot=10, a=22)
    planet.map[1, 0] = 0.5
    system = System([star, planet], scale=0)
    time = np.concatenate((np.linspace(0, 0.5, 10000, endpoint=False),
                           np.linspace(0.5, 4.5, 100, endpoint=False),
                           np.linspace(4.5, 5.5, 10000, endpoint=False),
                           np.linspace(5.5, 9.5, 100, endpoint=False),
                           np.linspace(9.5, 10, 10000)))
    phase = time / 10.
    system.compute(time)
    transit = phase[np.argmax(planet.z)]
    eclipse = phase[np.argmin(planet.z)]
    assert transit == 0
    assert eclipse == 0.5

    # Let's give the system a real length scale now
    system.scale = 1
    eps = 1e-3
    time = np.concatenate((np.linspace(0, 0.5, 100000, endpoint=False),
                           np.linspace(0.5, 5 - eps, 100, endpoint=False),
                           np.linspace(5 - eps, 5 + eps, 1000000,
                                       endpoint=False),
                           np.linspace(5 + eps, 9.5, 100, endpoint=False),
                           np.linspace(9.5, 10, 100000)))
    phase = time / planet.porb
    system.compute(time)
    transit = phase[np.argmin(np.abs(planet.x) + (planet.z < 0))]
    eclipse = phase[np.argmin(np.abs(planet.x) + (planet.z > 0))]
    assert transit == 0.999940999409994
    assert eclipse == 0.5000590894

    # Run some extra tests
    c = 299792458.
    RSUN = 6.957e8
    time = np.concatenate((np.linspace(0, 0.5, 100000, endpoint=False),
                           np.linspace(0.5, 4.5, 100, endpoint=False),
                           np.linspace(4.5, 5.5, 100000, endpoint=False),
                           np.linspace(5.5, 9.5, 100, endpoint=False),
                           np.linspace(9.5, 10, 100000)))
    phase = time / planet.porb
    system.scale = 1

    # Test this on an inclined orbit
    planet.inc = 30
    system.compute(time)
    transit = phase[np.argmax(planet.z)]
    eclipse = phase[np.argmin(planet.z)]
    delay_starry = (0.5 - (transit - eclipse)) * planet.porb * 86400
    a = planet.a * system.scale * RSUN
    delay_analytic = 2 * a / c * np.sin(planet.inc * np.pi / 180)
    assert np.abs(1 - delay_starry / delay_analytic) < 0.01

    # Test this on an orbit with nonzero Omega
    planet.inc = 90
    planet.Omega = 90
    system.compute(time)
    transit = phase[np.argmax(planet.z)]
    eclipse = phase[np.argmin(planet.z)]
    delay_starry = (0.5 - (transit - eclipse)) * planet.porb * 86400
    a = planet.a * system.scale * RSUN
    delay_analytic = 2 * a / c
    assert np.abs(1 - delay_starry / delay_analytic) < 0.01

    # Compute the light travel time from the secondary
    # eclipse to the barycenter for an eccentric orbit.
    # First, compute the phase of secondary eclipse
    # with no light travel time delay:
    planet.inc = 90
    planet.Omega = 0
    planet.w = 30
    planet.ecc = 0.25
    system.scale = 0
    system.compute(time)
    eclipse0 = phase[np.argmin(planet.z)]
    z0 = planet.z[np.argmin(planet.z)]
    # Now, compute the phase w/ the delay
    system.scale = 1
    system.compute(time)
    eclipse = phase[np.argmin(planet.z)]
    # Compute the light travel time to the barycenter in days:
    travel_time_starry = (eclipse - eclipse0) * planet.porb * 86400
    # Compute the analytic light travel time to the barycenter in days:
    travel_time_analytic = (0 - z0) * system.scale * RSUN / c
    assert np.abs(1 - travel_time_starry / travel_time_analytic) < 0.01


if __name__ == "__main__":
    test_light_travel()
