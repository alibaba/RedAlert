from setuptools import setup, find_packages
setup(
    name = 'raweb',
    version = '0.0.1',
    keywords = ('RedAlertWeb', 'raweb'),
    description = 'RedAlert Web Project',
    license = 'A License',

    install_requires = ['simplejson >= 3.3.0',
                        'sqlalchemy >= 0.8.7',
                        'werkzeug >= 0.8.3'],

    author = 'shenma',
    author_email = 'zhebin.xzb@alibaba-inc.com',
    packages = ['raweb'],
    include_package_data = True,
    platforms = 'any'
)
