<?php
  print '<?xml version="1.0" encoding="UTF-8"?>' . "\n";

  // Get the response object.
  $mediamosa_response = mediamosa::get();

  // Add our default response attribute.

  $mediamosa_version = mediamosa::get_environment_setting(mediamosa::ENV_MEDIAMOSA_VERSION);
  $mediamosa_response->add_response_attribute('version', $mediamosa_version[mediamosa_version::MAJOR] . '.' . $mediamosa_version[mediamosa_version::MINOR]);
  $mediamosa_response->add_response_attribute('xmlns:opensearch', 'http://a9.com/-/spec/opensearch/1.1/');

  // Get the header.
  $header = $mediamosa_response->get_response_header();

  // Set defaults.
  $header += array(
    'request_class' => '',
    'request_matched_method' => '',
    'request_matched_uri' => '',
  );
?>
<response<?php print drupal_attributes($mediamosa_response->get_reponse_attributes()); ?>>
  <header>
    <opensearch:itemsPerPage><?php print $mediamosa_response->item_count; ?></opensearch:itemsPerPage>
    <opensearch:totalResults><?php print $mediamosa_response->item_count_total; ?></opensearch:totalResults>
    <opensearch:startIndex><?php print $mediamosa_response->item_offset; ?></opensearch:startIndex>
    <request>
      <resultId><?php print mediamosa_unicode::xmlentities($header['request_result_id']); ?></resultId>
      <result><?php print mediamosa_unicode::xmlentities($header['request_result']); ?></result>
      <resultDescription><?php print mediamosa_unicode::xmlentities($header['request_result_description']); ?></resultDescription>
      <method><?php print mediamosa_unicode::xmlentities($this->request_method); ?></method>
      <uri><?php print (substr($this->request_uri, 0, 1) == '/' ? '' : '/') . mediamosa_unicode::xmlentities($this->request_uri); ?></uri>
      <time><?php print mediamosa_unicode::xmlentities($this->get_processed_time()); ?></time>
    </request>
<?php if (mediamosa_debug::is_debug()): ?>
    <debug>
      <class><?php print mediamosa_unicode::xmlentities($header['request_class']); ?></class>
      <mediamosaVersion><?php
        print $mediamosa_version[mediamosa_version::MAJOR] . '.' . $mediamosa_version[mediamosa_version::MINOR] . '.' . $mediamosa_version[mediamosa_version::RELEASE]
        ?></mediamosaVersion>
      <matched>
        <method><?php print mediamosa_unicode::xmlentities($header['request_matched_method']); ?></method>
        <uri><?php print mediamosa_unicode::xmlentities($header['request_matched_uri']); ?></uri>
      </matched>
<?php
      $debug_app = array('clientApplication' => array('appId' => mediamosa::get_environment_setting(mediamosa::ENV_APP_ID), 'appIds' => mediamosa::get_environment_setting(mediamosa::ENV_APP_IDS), 'timezone' => mediamosa::get_environment_setting(mediamosa::ENV_TIMEZONE)));

      print mediamosa_response::render_array2xml($debug_app, 3);

      if (empty($this->response['errors'])) {
        print '      <errors/>' . "\n";
      }
      else {
        print '      <errors>' . "\n";

        if (!empty($this->response['errors'])) {
          foreach ($this->response['errors'] as $error) {
            print '        <error>' . mediamosa_unicode::xmlentities($error) . '</error>' . "\n";
          }
        }

        print '      </errors>' . "\n";
      }
?>    </debug>
<?php endif; ?>
    <version><?php print mediamosa_version::MEDIAMOSA_VERSION; ?></version>
  </header>
<?php print $mediamosa_response->generate_xml_items(); ?>
</response>
