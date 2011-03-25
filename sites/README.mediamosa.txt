
// $Id: $

BASIC INSTALLATION GUIDE
------------------------

Each MediaMosa (2.x) installation is split up into multiple URLs. Where each
URL can be a different server. However, you can setup MediaMosa on one server,
although only recommended for small websites or for testing purposes.

The servers can be split up into these types;

admin     - This is the drupal website for maintenance (default map).
job       - A job server handles transcoding, creation of stills.
            (job1 / job2.mediamosa.local maps)
app       - A app server handles REST calls, for example seaching and retrieving
            data from the database is done using REST calls on these servers.
            (app1 / app2.mediamosa.local maps)
download  - A download server handles downloads of uploaded (media) files.
            (download.mediamosa.local map)
upload    - A upload server handles uploads of (media) files like video/audio.
            (upload.mediamosa.local map)

All types can be installed one server. However its good practice to use
different URLs for each type. In our example setup we use 2xapp and 2xjob
servers and we use the domain mediamosa.local. You are not required to use the
example and adjust for own needs. All types are required. Upload and download
can be combined and both can be used on the main website.

The job servers are handled by MediaMosa, the number of process slots should be
the same as number of processors in the server.

Using more than one app server requires http load balancing, having the load
balancer on app.mediamosa.local (example name) using app1 / app2 etc as child
servers.
 
Remember:
Job, app, download and upload are REST interfaces.
All types requires the complete installation.
Download and upload REST interfaces are restricted to only REST calls that are
related for download and/or upload.
Each settings.php re-uses the default settings.
Good practice is to study the example settings.php files.
Enabling app/job and admin on the same URL/server will create conflicts with 
/user uri of Drupal and REST calls that use /user in their URI. In most cases
its smart to have one admin and no other types on it. In 2.x its no longer
required to have multiple admins for each server type, all installations use
the same database and is no longer split up on job servers like it was in 1.x. 