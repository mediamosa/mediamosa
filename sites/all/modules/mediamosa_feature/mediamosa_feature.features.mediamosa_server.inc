<?php
/**
 * @file
 * mediamosa_feature.features.mediamosa_server.inc
 */

/**
 * Implements hook_mediamosa_servers().
 */
function mediamosa_feature_mediamosa_servers() {
  $mediamosa_servers = array();

  // Exported mediamosa server: Default server for stills.
  $mediamosa_servers['default_server_for_stills'] = array(
    'version' => 0,
    'server_status' => 'ON',
    'server_type' => 'STILL',
    'server_uri' => 'http://download.mediamosa.local/',
    'server_path' => '{base_uri}media/{TICKET}',
    'description' => 'Default server for stills',
    'containers' => '',
    'mime_type' => '',
    'object_code' => '',
    'server_name' => 'Default server for stills',
    'slots' => 0,
    'tools' => '',
    'uri_upload_progress' => '',
    'uri_upload_progress_server' => '',
    'uri_upload_progress_server_type' => 'APC',
    'machine_name' => 'default_server_for_stills',
  );

  return $mediamosa_servers;
}
