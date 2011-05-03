<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

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

  // Just here so ZEND analyser doesn't give warnings about $variables.
  $variables['ignore_zend'] = TRUE;

  $title = $variables['rest_call']->title;
  $description = $variables['rest_call']->description;
  $uri = $variables['rest_call']->uri;
  $method = $variables['rest_call']->method;
  $class = $variables['rest_call']->method;

  $classname = $variables['rest_call']->{mediamosa_rest_call::CLASS_NAME};
  $link_to_debug = module_exists('mediamosa_development') ? l(t('Debug REST call'), strtr('admin/mediamosa/config/development/set_rest_call/@uri/@method', array('@uri' => str_replace('/', '-', $uri), '@method' => $method))) . ', REST class: ' . $classname : '';


  $request_authorization = $variables['rest_call']->request_authorization;
  $warnings = $variables['warnings'];
  $example_request = is_array($variables['rest_call']->example_request) ? '<p>' . implode("</p>\n<p>", $variables['rest_call']->example_request) . '</p>' : $variables['rest_call']->example_request;
  $example_response = $variables['rest_call']->example_response;
?>
<div id="restcall">
<h2><?php print $title; ?></h2>
<p><?php print str_replace("\n\n", "</p>\n<p>", $description); ?></p>
<h3><?php print t('Request URL'); ?></h3>
<p><?php
  print t('@uri [@method]', array('@uri' => '/' . $uri, '@method' => $method));
  if (!empty($link_to_debug)) {
    print ' <small>(' . $link_to_debug . ')</small>';
  }
?></p>
<h3><?php print t('Request Authorization'); ?></h3>
<p><?php print $request_authorization; ?></p>

<?
  // Set any warning.
  if (!empty($warnings)) {
?>
<h3><?php print t('Warnings and checks during page generation'); ?></h3>
<ul><li><?php print implode('<li>', $warnings); ?></ul>
<?
  }
?>
<h3><?php print t('Request Parameters'); ?></h3>
<?php print $variables['parameters']; ?>
<?php if (!empty($variables['response_fields'])) { ?>
<h3><?php print t('Response fields'); ?></h3>
<?php print $variables['response_fields']; ?>
<?php } ?>
<h3><?php print t('Example Request'); ?></h3>
<p><?php print str_replace("\n\n", "</p>\n<br>\n<p>", $example_request); ?></p>
<h3><?php print t('Example Response'); ?></h3>
<pre><?php print htmlentities($rest_call->example_response); ?></pre>
</div>
