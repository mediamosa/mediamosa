Instructions on installing & configuring SOLR for MediaMosa
===========================================================

Requirements;

- Java Development Kit (JDK) 1.5 or later.
- Any java EE servlet engine app-server. Jetty is included in solr,
  you can use apache-tomcat as well.

* Install JDK:
sudo apt-get install sun-java6-jdk

* Install lib mysql for java
sudo apt-get install libmysql-java

* obtain Solr from http://lucene.apache.org/solr/

MediaMosa currently works without errors with version 1.4.1 of
apache-solr. Version 3.5 (latest at the moment of writing) is known to
have issues.

for example:

mkdir /srv/solr
cd /srv/solr
wget http://www.apache.org/dist//lucene/solr/1.4.1/apache-solr-1.4.1.tgz
tar xzvf apache-solr-1.4.1.tgz
ln -s apache-solr-1.4.1 apache-solr
cd apache-solr

* copy schema's from MediaMosa to SOLR

  cp -r example/ mediamosa
  cd mediamosa/solr/conf/
  cp <mediamosa_root>/sites/all/modules/mediamosa_solr/*.xml .

Alternative for the last step: you could man a symlink to the mediamosa source so 
that this also gets update when MediaMosa gets an update:

  ln -s <mediamosa_root>/sites/all/modules/mediamosa_solr/*.xml .


To host the Solr java servlet use either tomcat or jetty, this
document only describes jetty. Jetty comes preconfigured in the Solr
tar.

---- Manually testing the SOLR installation ----------------------------

Go into the extracted solr folder and attempt a manual start by:

cd /srv/solr/apache-solr/mediamosa/
java -jar start.jar

This command will be very verbose but mention the following in the last
20 lines:

"Started SocketConnector @ 0.0.0.0:8983"

This indicates a functioning SOLR package.

Whilst SOLR is running you should be able to access the SOLR admin page
on URL: http://<mediamosa_solr_url>:8983/solr/

=== End jetty ==========================================================



---- Configuring MediaMosa for SOLR usage ------------------------------
I. Go to the Modules section and enable the following module:
URL: https://<mediamosa_url>/admin/modules

Module name: "MediaMosa Apache Solr"

II. Go to MediaMosa administration page, and select Configuration and
then "MediaMosa Solr" URL:
https://<mediamosa_url>/admin/mediamosa/config/solr

Enter the correct URL for the Solr instance.

Example: http://<mediamosa_solr_url>:8983/solr/

Use 'Check connection Solr' to verify the connection is ok.

------------------------------------------------------------------------



---- Configuring SOLR to run as daemon (only for jetty!) ---------------

To start as daemon: 

start-stop-daemon -b -m -p /var/run/solrservice.pid --chuid <username> \
  --start --quiet --exec /usr/bin/java -d<path_to_solr>/solr -- \
  -jar start.jar

Please replace the variable <username> to the user with sufficient rights
Also replace <path_to_solr> with the used install path to Solr

To stop the proces:
start-stop-daemon --stop --quiet -p /var/run/solrservice.pid

------------------------------------------------------------------------

* Done!
