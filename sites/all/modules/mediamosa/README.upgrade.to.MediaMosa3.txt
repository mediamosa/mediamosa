
Upgrading to MediaMosa 3.1
--------------------------

Before you upgrade, make a backup!

Read http://drupal.org/upgrade for more general information on how to
upgrade a Drupal site. Do not forget to run update.php after updating
the MediaMosa codebase.

Upgrading from 3.0 will keep the existing transcode profiles and
server object codes in place. If you want the newer versions, it is
recommended you do a new test installation and review the new profiles
and object players.

Also version 3.1 adds a /media alias and a new still download server
definition pointing to this alias. This ensures a better performance
for stills in MediaMosa, so it is advised to do so.

add the next to your apache setup:
    # Media
    Alias /media [mount_point]/media
    <Directory [mount_point]/media>
      Options FollowSymLinks
      AllowOverride All
      Order deny,allow
      Allow from All
    </Directory>

And go in the Admin site to MediaMosa-Configuration-Servers, edit the
STILL server and change 'PATH'-field. Change the /still to /media, to
for example: {base_uri}media/{TICKET}


Upgrading to MediaMosa 3
------------------------

You can only upgrade from MediaMosa 2.2 or higher to MediaMosa 3. All versions
below 2.2 are Release Candidates and will not have a complete upgrade path to
any version higher than MediaMosa 2.2.

Upgrading from MediaMosa 1.7.x
-------------------------------
Upgrading from MediaMosa 1.7.x will require you to create a new MediaMosa
installation and migration of the database to 3.x.

Before you upgrade from 1.7.x, make a test environment of your current version
or make a backup!

You won't need the 1.7 source anymore, you will need to replace it with the 3.x
version. Setup the /sites/default/settings.php, following instructions from
Drupal here; http://drupal.org/documentation. 

MediaMosa 1.x was made with Drupal 6, MediaMosa 2 and 3 are made with Drupal 7.

Once you have setup setting.php, then browse to install.php and install
MediaMosa. During installation, lots of information is given about installation
requirements and settings for Apache. Also short guide how to migrate from 1.7.

Once installed, click on 'Configuration' on the top right and select the
Migration tool. The migration tool will do some basic tests, if these are 
successful, the migration can be started. If not, follow the instructions on 
what is wrong.

Once the migration has started, you can't restart it anymore. You will need to
re-install MediaMosa 3 and restart the migration again.

Your 1.7.x databases will not be changed, and its wise to keep backups(!).

The migration will only migrate data that is used by MediaMosa. It will not
migrate the Drupal 6 installation to Drupal 7. This means Drupal users, Drupal
roles, modules that are not used by MediaMosa etc. So if you added users, roles,
modules etc. to the previous 1.7.x installation, it will not be converted to the 
MediaMosa 3 installation.

Upgrading from MediaMosa 2.2 or higher
--------------------------------------

The upgrade from version 2.x to version 3 is very easy!

1. Backup your current installation, thats the www root and your 'mediamosa'
database.
2. Remove your current www root and replace it with the new MediaMosa 3.x 
installation. MediaMosa 3.x will have a lot of files that are either removed or
renamed. If you copy the new version over the 2.x version, you will still keep
lot of files that are no longer in 3.x. So its important that your installation
is new and fresh.
3. Copy /sites/default/settings.php from your 2.x backup. Also any other
settings.php files in your /sites/[sitemap]/settings.php should be copied. So if
[2.x backup]/sites/app1.myhost/settings.php exists in 2.x, then copy it to the 
new 3.x version, same place, same directory ([3.x install]/sites/app1.myhost/).
 What you should never copy from your backup is /sites/all/* directory.
4. Unless you have installed other modules in the 2.x installation, you can copy
these now or do those later.
5. Now go directly to /update.php on your MediaMosa (admin) website. This will
start the update from 2.x to 3.x. Most common error that you are not allowed to
update, is because you are not logged in. You can fix this by following the
instructions on the page about enabling $conf['update_free_access'] in
your settings.php. Follow these instruction carefully.
6. Once access is gained, press 'continue' and press 'apply pending updates'.

And you're done!
