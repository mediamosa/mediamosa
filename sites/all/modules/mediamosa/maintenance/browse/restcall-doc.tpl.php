<?php
/**
 * @file
 * Default theme implementation to show REST call documentation.
 *
 * Available variables:
 * - $title: title of rest call.
 * - $description: Description of the REST call.
 * - $uri: The uri of the REST call.
 * - $method: The method of the REST call (GET/POST)
 * - $request_authorization: Text if autorization is required.
 * - $warnings: possible warnings during page creation.
 * - $example_request: A example uri of a request.
 * - $example_response: A example response XML.
 *
 * @see template_preprocess()
 * @see template_preprocess_restcall_doc()
 */

  $title = $variables['rest_call']->title;
  $description = $variables['rest_call']->description;
  $uri = $variables['rest_call']->uri;
  $method = $variables['rest_call']->method;

  $classname = $variables['rest_call']->{mediamosa_rest_call::CLASS_NAME};
  $link_to_debug = module_exists('mediamosa_development') ? l(t('Debug REST call'), strtr('admin/mediamosa/tools/development/set_rest_call/@uri/@method', array('@uri' => str_replace('/', '-', $uri), '@method' => $method))) . ', REST class: ' . $classname : '';


  $request_authorization = $variables['rest_call']->request_authorization;
  $warnings = $variables['warnings'];
  $example_request = is_array($variables['rest_call']->example_request) ? '<p>' . implode("</p>\n<p>", check_plain($variables['rest_call']->example_request)) . '</p>' : check_plain($variables['rest_call']->example_request);
  $example_response = $variables['rest_call']->example_response;
?>
<div id="restcall-details">
<p><h2><?php print $title; ?></h2>
<?php print str_replace("\n\n", "</p>\n<p>", $description); ?></p>
<p><h3><?php print t('Request URL'); ?></h3>
<?php
  print t('@uri [@method]', array('@uri' => '/' . $uri, '@method' => $method));
  if (!empty($link_to_debug)) {
    print ' <small>(' . $link_to_debug . ')</small>';
  }
?>
</p>
<p>
<h3><?php print t('Request Authorization'); ?></h3>
<?php print $request_authorization; ?>
</p>
<?php
  // Set any warning.
  if (!empty($warnings)) {
?>
<p>
<h3><?php print t('Warnings and checks during page generation'); ?></h3>
<ul><li><?php print implode('<li>', $warnings); ?></ul>
</p>
<?php
  }
?>
<h3><?php print t('Request Parameters'); ?></h3>
<?php print $variables['parameters']; ?>

<?php if (!empty($variables['parameters_internal'])): ?>
<h3><?php print t('Internal Request Parameters'); ?>&nbsp;<small>(can only be used during internal REST calls)</small></h3>
<?php print $variables['parameters_internal']; ?>
<?php endif; ?>

<?php if (!empty($variables['response_fields'])): ?>
<h3><?php print t('Response fields'); ?></h3>
<?php print $variables['response_fields']; ?>
<?php endif; ?>
<h3><?php print t('Example Request'); ?></h3>
<p><?php print str_replace("\n\n", "</p>\n<br>\n<p>", $example_request); ?></p>
<h3><?php print t('Example Response'); ?></h3>
<pre><?php print htmlentities($example_response); ?></pre>
</div>
