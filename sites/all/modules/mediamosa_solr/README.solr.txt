
How to install Solr (Unbutu 10.04), 'the quick short version'.

Requirements;

- Java Development Kit (JDK) 1.5 or later.
- Any java EE servlet engine app-server. (jetty is included in solr). 
  Will be using apache-tomcat instead.


* do update & upgrade
sudo apt-get update
sudo apt-get upgrade

 * Install JDK:
sudo apt-get install sun-java6-jdk

* Install lib mysql for java
sudo apt-get install libmysql-java

* Install tomcat6 (Java Servlet)
sudo apt-get install tomcat6 tomcat6-admin tomcat6-common tomcat6-user tomcat6-docs tomcat6-examples

* Download apache-solr-1.4.1.tar.gz (or higher) from http://lucene.apache.org/solr/
wget http://apache.proserve.nl/lucene/solr/1.4.1/apache-solr-1.4.1.tgz
tar -xf apache-solr-1.4.1.tgz

* Copy solr java file to tomcat6 server
sudo cp apache-solr-1.4.1/dist/apache-solr-1.4.1.war /usr/share/tomcat6/webapps/.

* Copy basic setup solr to tomcat6 server
sudo cp -R apache-solr-1.4.1/example/solr/ /usr/share/tomcat6/solr/

* Create a Solr configuration for Tomcat6;
sudo pico /etc/tomcat6/Catalina/localhost/solr.xml

<Context docBase="/usr/share/tomcat6/webapps/apache-solr-1.4.1.war" debug="0" privileged="true" allowLinking="true" crossContext="true">
<Environment name="solr/home" type="java.lang.String" value="/usr/share/tomcat6/solr" override="true" />
</Context>

* Enable multicore in Solr 1.4.1
sudo cp apache-solr-1.4.1/example/multicore/solr.xml /usr/share/tomcat6/solr/solr.xml
sudo cp -R apache-solr-1.4.1/example/multicore/core0 /usr/share/tomcat6/solr/
sudo cp -R apache-solr-1.4.1/example/multicore/core1 /usr/share/tomcat6/solr/
sudo mkdir /usr/share/tomcat6/solr/core0/data
sudo mkdir /usr/share/tomcat6/solr/core1/data
sudo chown -R tomcat6:tomcat6 /usr/share/tomcat6/solr/core0/data/
sudo chown -R tomcat6:tomcat6 /usr/share/tomcat6/solr/core1/data/

* Restart tomcat6;
sudo /etc/init.d/tomcat6 restart

* Done!
