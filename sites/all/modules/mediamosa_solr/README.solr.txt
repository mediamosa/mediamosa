
Instructions on installing & configuring SOLR for MediaMosa
===========================================================

Requirements;

- Java Development Kit (JDK) 1.5 or later.
- Any java EE servlet engine app-server. (jetty is included in solr).
  Can use apache-tomcat aswel.

* do update & upgrade
sudo apt-get update
sudo apt-get upgrade

 * Install JDK:
sudo apt-get install sun-java6-jdk

* Install lib mysql for java
sudo apt-get install libmysql-java

* Install tomcat6 (Java Servlet) (Optional)
sudo apt-get install tomcat6 tomcat6-admin tomcat6-common tomcat6-user tomcat6-docs tomcat6-examples

* obtain Solr from http://lucene.apache.org/solr/ or the MediaMosa preconfigured
from MadCap.

Extract the MediaMosa solr archive (tar -xf apache-solr-mm-3.3.0.tgz)

To host the Solr java servlet use either tomcat or jetty, both are descibed below;

=== For tomcat server only!  ===========================================

* Copy solr java file to tomcat6 server
sudo cp apache-solr-mm-3.3.0/dist/apache-solr-3.3.0.war /usr/share/tomcat6/webapps/.

* Copy basic setup solr to tomcat6 server
sudo cp -R apache-solr-mm-3.3.0/example/solr/ /usr/share/tomcat6/solr/

* Create a Solr configuration for Tomcat6;
sudo pico /etc/tomcat6/Catalina/localhost/solr.xml

<Context docBase="/usr/share/tomcat6/webapps/apache-solr-3.3.0.war" debug="0" privileged="true" allowLinking="true" crossContext="true">
<Environment name="solr/home" type="java.lang.String" value="/usr/share/tomcat6/solr" override="true" />
</Context>

* Enable multicore in Solr 3.3.0
sudo cp apache-solr-3.3.0/solr/multicore/solr.xml /usr/share/tomcat6/solr/solr.xml
sudo cp -R apache-solr-3.3.0/solr/multicore/core0 /usr/share/tomcat6/solr/
sudo cp -R apache-solr-3.3.0/solr/multicore/core1 /usr/share/tomcat6/solr/
sudo mkdir /usr/share/tomcat6/solr/core0/data
sudo mkdir /usr/share/tomcat6/solr/core1/data
sudo chown -R tomcat6:tomcat6 /usr/share/tomcat6/solr/core0/data/
sudo chown -R tomcat6:tomcat6 /usr/share/tomcat6/solr/core1/data/

* Restart tomcat6;
sudo /etc/init.d/tomcat6 restart

=== End tomcat server configuration ====================================



=== For jetty servlet hosting instead of tomcat ========================

Jetty comes preconfigured in the Solr tar, so no configuration is required.

---- Manually testing the SOLR installation ----------------------------

Go into the extracted solr folder /solr and attempt a manual start by:
java -jar start.jar

This command will be very verbose but mention the following in the last
20 lines:

"Started SocketConnector @ 0.0.0.0:8983"

This indicates a functioning SOLR package.

Whilst SOLR is running you should be able to access the SOLR admin page
on URL: http://MACHINENAME:8983/solr/

=== End jetty ==========================================================



---- Configuring MediaMosa for SOLR usage ------------------------------
I.
Go to the Modules section and enable the following module:
URL: https://beheer.acceptatie.vpcore.snkn.nl/admin/modules

Module name: "MediaMosa Apache Solr"

II.
Go to MediaMosa administration page, and select Configuration and then "MediaMosa Solr"
URL: https://beheer.acceptatie.vpcore.snkn.nl/admin/mediamosa/config/solr

Enter the correct URL for the Solr instance.
Example: http://db2.acceptatie.vpcore.snkn.nl:8983/solr/

Use 'Check connection Solr' to verify the connection is ok.

------------------------------------------------------------------------



---- Configuring SOLR to run as daemon (only for jetty!) ---------------

To start as daemon:
start-stop-daemon -b -m -p /var/run/solrservice.pid --chuid <username> --start --quiet  --exec /usr/bin/java -d<path_to_solr>/apache-solr-3.3.0/solr -- -jar start.jar
Please replace the variable <username> to the user with sufficient rights
Also replace <path_to_solr> with the used install path to Solr

To stop the proces:
start-stop-daemon --stop --quiet -p /var/run/solrservice.pid

------------------------------------------------------------------------

* Done!