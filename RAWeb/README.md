Red Alert Web
=============
Introduction
------------
Red Alert is an open source, browser based cluster monitoring system. Users could configure red alert backend on browser, like shielding policies and deploying trendence factor, as well as grasping backends' status and infomation.

Requirements
------------
* Red Alert Backend
* Red Alert Web package
* Data Source like [Graphite](https://graphite.readthedocs.io/en/latest/)

Installation
------------
* Get Code: `git clone http://gitlab.alibaba-inc.com/red_alert/red_alert_web.git`

Configuation
------------

***common configuation***   

Red Alert Web could be started in three ways(apache, uwsgi and location). No matter which way, *conf/red_alert_web.conf* and *static/ra_conf.js* must be modified. *conf/red_alert_web.conf* looks like this.

    [DEFAULT]    
    projectRoot = /var/www/html/red_alert_web    
    workspace = /var/www/html/red_alert_web    
    
    [PathConfig]   
    RedAlertWebWorkRoot = %(workspace)s/work   
    RedAlertWebConfDir = %(RedAlertWebWorkRoot)s/current   
    RedAlertWebTablePath = %(RedAlertWebConfDir)s/sqlite   
    RedAlertWebAuxPath = %(RedAlertWebWorkRoot)s/raweb.aux.db   
    RedAlertWebJsonPath = %(RedAlertWebWorkRoot)s/raweb.json   
    RedAlertWebVersionDir = %(RedAlertWebWorkRoot)s/tmp   
    
    [Parameter]   
    runPort = 50007   
    MaxRaBackEnd = 3   
    RedAlertStoragePath = file://var/www/html/foo   
    
    [fsLib]   
    fsUtil = %(projectRoot)s/fs_lib/bin/fs_util

`DEFAULT` section defines global variables named `projectRoot` and `workspace`. `projectRoot` is web's root directory while `workspace` could be specified to any path with permission( If apache has been used, please make sure apache group has the premission).   

As long as `DEFAULT` section has been configued, `Parameter` section would be generated automatically. If workspace doesn't exist, it would be created when first run.   

- `RedAlertWebWorkRoot` is Red Alert Web's workspace home.   

- `RedAlertWebConfDir` is Red Alert Web's `sqlite` file directory. This directory saves `sqlite` file.

- `RedAlertWebTablePath` is `sqlite`'s absolute path. The `sqlite` contains `Policy`, `RedAlert`, `DataSource`, `Pairs`, `Shield` tables, those would be used in backends.   

- `RedAlertWebAuxPath` is `raweb.aux.db`'s absolute path which is a database file as well, but only be used in web self. `raweb.aux.db` records `sqlite` table's modified status.   

- `RedAlertWebJsonPath` is `raweb.json`'s absolute path, which is used for verify input string.   

- `RedAlertWebVersionDir` is web's version directory which records `sqlite`'s release. If policies or data sources were modified and deployed, a new `sqlite` release would be produced in *RedAlertWebVersionDir*.   

- `runPort` is an server listening port, that only be neccessory in local script running. If deployed on Apache or run with `uwsgi`, port would be assigned in those server's configuration, just ignore this option.   

- `MaxRaBackEnd` is the max quantity of backend permitted.   

- `RedAlertStoragePath` is a remote filesystem storage path. Red Alert supports local filesystem and pangu distributed filesystem now, please prefixed to ```file://``` or ```pangu://``` and make sure this directory could be accessed. If pangu has been installed, `fsUtil` is panggu's binary path or script, it's neccessary to specify.

***Static Confiuation***   

`static` directory provide static files. Modify `ra_conf.js` file, `api_url` should be specifid to your `http://<ip>:<port>` address or url. It looks like this  

    var ra_conf = {     
    	"api_url": "http://localhost:5011",   
    	"current_url": "http://0.0.0.0:5011/index.html",   
    	"buc_sso_url": "http://search-tools.yisou.com/buc_sso/index.php",   
    	"api_timeout": 3000,   
    	"admin_timeout": 10000,   
    	"disable_account": true   
	};

*NOTE:* If you don't need authentication, please set `disable_account` option true. As long as `false` setted, `buc_sso_url` works to verify identidy.
  
Running
-------
OK now, All basical configuations have been done, run it by `python raweb/main.py`, and then point your browser at `http://localhost:50007/index.html`.   

***uwsgi***

If you prefer to uwsgi server, we provide [uwsgi1.4](https://uwsgi-docs.readthedocs.io/en/latest/WSGIquickstart.html), just enter `uwsgi` dirctory and `make` it. You should modify `raweb_install_prefix` and `http` options  in `raweb.ini`, or create your own `.ini` file. The default `raweb.ini` looks like this

    [uwsgi]
    raweb_install_prefix = /var/www/html/red_alert_web
    check-static = %(raweb_install_prefix)s/static
    daemonize2 = ./uwsgi.log
    http = localhost:50007
    log-maxsize = 1024000
    wsgi-file = %(raweb_install_prefix)/raweb/raweb.wsgi
    master = true
    workers = 1
    threads = 10
    pidfile = ./uwsgi.pid

Creating your own `*.ini` conf file is a better way, [Quickstart for Python Application](https://uwsgi-docs.readthedocs.io/en/latest/WSGIquickstart.html). Running `./uwsgi/uwsgi --ini raweb.ini` to start web service. 

***Apache***

Web service could alse be deployed on [Apache Server](https://httpd.apache.org/docs/2.2/).If you never run python program on Apache,you should install [mod-wsgi](http://werkzeug.pocoo.org/docs/0.11/deployment/mod_wsgi/) first. We provide a template, on `conf/` dir for reference which looks like: 

    Listen localhost:5011
    <VirtualHost localhost:5011>
        ServerName www.raweb.com
		   DocumentRoot /var/www/html/red_alert_web
		   WSGIDaemonProcess red_alert_web python-path=/usr/ali/lib/python2.5/site-packages/:/var/www/html/red_alert_web
		   WSGIScriptAlias / /var/www/html/red_alert_web/raweb/raweb.wsgi process-group=red_alert_web application-group=%{GLOBAL}
		   CustomLog /var/log/httpd/raweb_access.log common
		   ErrorLog /var/log/httpd/raweb_error.log
		 <Directory /var/www/html/red_alert_web>
		 	      Order allow,deny
		   	      Allow from all
    		</Director>
	</VirtualHost>

*Note!* Apache's group should hve been authorized on this Directory. Maybe, `sudo chmod www-data:www-data your-root`.   
 
Move the `.conf` to `/etc/httpd/conf.d`, then restart your httpd service.

***Apache2***

Apache2's configuation is similar to Apache, Getting `mode-wsgi` first, then move your `*.conf` to `/etc/apache2/sites-avaliable` and create an symbolic link in `sites-enabled`.

Quick Start
-----------

You're up and running! Red Alert Web is now running.

The first screen you arrived at is *policy configuation*, where policies could be editted here. In *shield scrren*, policies in specified machines could be shielded in specified time. `data source` in miscellaneous configuration should be added at least one when first run, without that backends can't work. Now backends support `Graphite` and `Amonitor`, but you can add other data sources as well, just implement `fetchMetrics` interface. In *deploy screen*, you could see tables' modifications. Online backend's infomation and status would be shown on `console` screen as well as `sqlite` releases and `storagePath`, In *admin screen*. Pay attention to the *Explanation*.  
 
- *Refresh Status* : Refresh and read web's current status immediately.   

- *deploy* : All modifications saved in `aux.raweb.db` will not work, unless you deploy. Deploy will create a new `sqlite` version.   

- *reload current* : If backends work with multiple releases of sqlite, or other accident happened, reload the latest version. All backends will receive the latest information and reload it.   

- *recover* : Clean `raweb.aux.db`. If recovered, all tables in `raweb.aux.db` will be cleaned, that means all changes without deployed will disappear.   

- *reload* : If you want to rollback to old version, points to *reload*. Reload option will rollback all tables except `redAlert`, course outdate backend infomation is uesless and dangerous.

Contribute
----------