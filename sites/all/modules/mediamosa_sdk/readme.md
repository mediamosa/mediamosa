MediaMosa Software Development Kit
==================================

The MediaMosa SDK has been developed for Drupal 6 and Drupal 7. These PHP API tools will enable developers to connect front-end applications (like websites) to the back-end (MediaMosa).

There are slight differences between Drupal 6 and Drupal 7 versions of the SDK. Although the *.class.inc are the same on all version.


The files
---------

- mediamosa_sdk.class.inc (6.x / 7.x)
  This file contains the main class and contants used by front-end and back-end application.

- mediamosa_sdk.* (6.x / 7.x)
  These files are Drupal related.

- mediamosa_connector/* (6.x / 7.x)
  This is the connector module. You can reuse the mediamosa_connector.class.inc and mediamosa_connector.response.class.inc in your own code when not using the Drupal code.

  - mediamosa_connector.class.inc
    This class is your connector class that will allow your code to connect and execute REST calls on MediaMosa REST interface(s).

  - mediamosa_connector.response.class.inc
    This object is return each time you execute a REST call.

- mediamosa_development/* (6.x / 7.x)
  The mediamosa_development Drupal module allows developers to test and run REST calls directly using a form.

- mediamosa_restcalls_post2get/* (7.x)
  This Drupal module maps all POST REST call to /post2get/[rest call] for GET method. Allows you to use POST calls as GET.


Usage
-----
$my_mediamosa_connector = new mediamosa_connector('testapp', 'password', 'http://app1.mediamosa.local');

try {
  $response = $my_mediamosa_connector->request('version', array('fatal' => TRUE)); 
}
catch (Exception $e) {
  // .. something went wrong.
  // Do some handeling of the exception here and don't continue for this point.
}

