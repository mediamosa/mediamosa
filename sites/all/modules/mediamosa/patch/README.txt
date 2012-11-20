Notes on .patch file(s).

These patch files are applied on Drupal 7.2 and are required in some cases.
MediaMosa is released with Drupal 7.2 and has these patches applied. These
patches are used for own Drupal 7 versions, when needed for upgrading Drupal.
However remember that its best to use the Drupal version supplied with
MediaMosa.

- bootstrap-cookie.patch;

  -- Cookie domain --
  This patch is required for the cookie domain fix and the fix on simpletest.
  The cookie domain fix will fix problems on the 'www.' subdomain usage of
  Drupal. Patch will allow you to use cookies on different subdomains.
  For testing the patch, include the following line in the mediamosa.info file;

  files[] = patch/mediamosa_cookie_domain.test

- bootstrap-simpletest.patch;

  -- Simpletest loadbalancer / multi server limitation --
  Drupal 7 has problems when running simpletest on loadbalancers when tests do
  HTTP requests inside a simpletest. When unit tests use HTTP requests during
  the test, all requests that are made to other MediaMosa instances (like job
  servers) will fail if the target server does not have the same file root as
  the caller. This is caused by Drupals security on incoming HTTP calls from
  unit tests. The call fails because the security validation includes an unique
  file ID of the calling server's bootstrap.inc, which is different on other
  servers installation. Therefor the call is not accepted and will fail. To
  solve this problem, our patch disables this security layer.

  However, a word caution; this patch is intended for running simpletest on
  installations like acceptation/staging/test servers. Do not deploy this patch
  on production servers, as it will create an security vulnerability on your
  server(s).

