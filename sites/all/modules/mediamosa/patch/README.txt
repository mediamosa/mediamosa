Notes on .patch file(s).

These patch files are applied on Drupal 7.2 and are required in some cases.
MediaMosa is released with Drupal 7.2 and has these patches applied. These
patches are used for own Drupal 7 versions, when needed for upgrading Drupal.
However remember that its best to use the Drupal version supplied with
MediaMosa.

- bootstrap.patch;

  Cookie domain
  This patch is required for the cookie domain fix and the fix on simpletest.
  The cookie domain fix will fix problems on the 'www.' subdomain usage of
  Drupal. Patch will allow you to use cookies on different subdomains.

  Simpletest loadbalancer problem;
  Drupal 7 has problems when running simpletest on loadbalancers while tests can
  do HTTP requests inside a simpletest. When during the HTTP request inside the
  test is quering the loadbalancer, the other server might be chosen and
  resulting in a 403 (forbidden) http error. Other problems will occur on some
  tests which will do request on different REST interfaces on different servers.
