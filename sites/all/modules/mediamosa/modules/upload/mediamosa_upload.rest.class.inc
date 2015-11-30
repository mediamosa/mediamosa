<?php
/**
 * @file
 * REST functions for upload.
 */

/**
 * URI: /upload/ticket/create
 *
 * Method: POST
 */
class mediamosa_rest_call_upload_ticket_create extends mediamosa_rest_call {

  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const USER_ID = 'user_id';
  const GROUP_ID = 'group_id';
  const REFERENCE_ID = 'reference_id';
  const PROVIDER_ID = 'provider_id';
  const PUBLISHED = 'published';
  const TAG = 'tag';
  const IS_ORIGINAL_FILE = 'is_original_file';
  const IS_DOWNLOADABLE = 'is_downloadable';
  const COUNT = 'count';
  const ASSET_ID = 'asset_id';
  const USE_EXISTING_MEDIAFILE = 'use_existing_mediafile';

  // ------------------------------------------------------------ Get Var Setup.
  /**
   * Implements get_var_setup().
   */
  public function get_var_setup() {

    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $publish_default = 'TRUE';
    if ($app_id) {
      $app = mediamosa_app::get_by_appid($app_id);
      if (!empty($app)) {
        $publish_default = $app[mediamosa_app_db::PUBLISHED_ASSET_DEFAULT];
      }
    }

    $var_setup = array(
      self::VARS => array(
        self::USER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_USER_ID,
          self::VAR_DESCRIPTION => 'The user ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
          self::VAR_RANGE_END => mediamosa_user_db::NAME_LENGTH,
        ),
        self::GROUP_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_GROUP_ID,
          self::VAR_DESCRIPTION => 'The user group ID.',
          self::VAR_RANGE_END => mediamosa_user_group_db::GROUP_ID_LENGTH,
        ),
        self::REFERENCE_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The reference ID.',
          self::VAR_RANGE_END => mediamosa_asset_db::REFERENCE_ID_LENGTH,
        ),
        self::PROVIDER_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The provider ID.',
          self::VAR_RANGE_END => mediamosa_asset_db::PROVIDER_ID_LENGTH,
        ),
        self::PUBLISHED => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "The default published value of the asset. This default is overridden by the client application default, which is by default 'TRUE'.",
          self::VAR_DEFAULT_VALUE => $publish_default,
        ),
        self::IS_DOWNLOADABLE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => 'Mediafile can be downloaded.',
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
        self::TAG => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'User specified Mediafile tag.',
          self::VAR_RANGE_END => mediamosa_asset_mediafile_db::TAG_LENGTH,
        ),
        self::COUNT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_INT,
          self::VAR_DESCRIPTION => "The number of uploadtickets to create, default 1.",
          self::VAR_DEFAULT_VALUE => 1,
        ),
        self::ASSET_ID => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ASSET_ID,
          self::VAR_DESCRIPTION => "Instead of creating new assets, use the given asset_id's. Conflicts with the count parameter. Default empty.",
          self::VAR_IS_ARRAY => self::VAR_IS_ARRAY_YES,
        ),
        self::USE_EXISTING_MEDIAFILE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_BOOL,
          self::VAR_DESCRIPTION => "When using asset_id[], and if there is an empty mediafile, use that, otherwise create new mediafiles. Default is FALSE, and always create new mediafiles.",
          self::VAR_DEFAULT_VALUE => 'FALSE',
        ),
      ),
    );

    // Enrich with required REST vars.
    return self::get_var_setup_default($var_setup);
  }

  // ------------------------------------------------------------------ Do Call.
  /**
   * Implements do_call().
   */
  public function do_call() {
    $mediamosa = mediamosa::get();

    // Get the app_id.
    $app_ids = $this->get_param_value_app();
    $app_id = reset($app_ids);

    $owner_id = $this->get_param_value(self::USER_ID);
    $group_id = $this->get_param_value(self::GROUP_ID);
    $reference_id = $this->get_param_value(self::REFERENCE_ID);
    $provider_id = $this->get_param_value(self::PROVIDER_ID);
    $published = $this->get_param_value(self::PUBLISHED);
    $is_downloadable = $this->get_param_value(self::IS_DOWNLOADABLE);
    $tag = $this->get_param_value(self::TAG);
    $count = $this->get_param_value(self::COUNT);
    $asset_ids = $this->get_param_value(self::ASSET_ID);
    $use_exisiting_mediafile = $this->get_param_value(self::USE_EXISTING_MEDIAFILE);

    // Test webservice.
    mediamosa_webservice_app::webservice_must_be_active(mediamosa_webservice_app::HANDLE_MEDIA_MANAGEMENT, $app_ids);

    // Prepare fields.
    $is_downloadable = is_bool($is_downloadable) ? ($is_downloadable ? 'TRUE' : 'FALSE') : $is_downloadable;
    // Fields to insert in Mediafile.
    $fields = array(
      self::GROUP_ID => $group_id,
      self::IS_DOWNLOADABLE => $is_downloadable,
      self::TAG => $tag,
      self::IS_ORIGINAL_FILE => 'TRUE',
    );

    // Get serverupload.
    $server_upload = mediamosa_server::get_random_upload();

    if (!$server_upload) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_SERVER_UPLOAD_NOT_FOUND);
    }
    $url = mediamosa_http::expectHTTP($server_upload[mediamosa_server_db::SERVER_URI]);

    if (isset($asset_ids) && count($asset_ids) > 0) {
      $count = count($asset_ids);
    }

    // We disable direct updating of solr, force queue.
    if (class_exists('mediamosa_solr') && method_exists('mediamosa_solr', 'forceQueue')) {
      mediamosa_solr::forceQueue(TRUE);
    }

    for ($i = 1; $i <= $count; $i++) {
      if (isset($asset_ids) && count($asset_ids) > 0) {
        $asset_id = $asset_ids[$i - 1];
        try {
          mediamosa_asset::must_exists($asset_id);
        }
        catch (Exception $e) {
          // Does not exists so skip this asset_id.
          continue;
        }
      }
      else {
        // Create the asset, and do not reindex yet.
        $asset_id = mediamosa_asset::create($app_id, $owner_id, $group_id, $reference_id, $provider_id, $published);
      }

      $mediafile_id = FALSE;
      if ($use_exisiting_mediafile) {
        // Look for empty mediafiles in this asset.
        $mediafile_id = mediamosa_asset_mediafile::get_empty_mf_by_asset_id($asset_id);
      }
      if ($mediafile_id == FALSE) {
        // Create new mediafile.
        $mediafile_id = mediamosa_asset_mediafile::create(mediamosa_db::uuid($app_id), $app_id, $asset_id, $owner_id, $fields);
      }

      // Create Ticket.
      $ticket_id = mediamosa_media_ticket::create_for_upload($app_id, $mediafile_id, $owner_id, $group_id);

      // Generate unique ID.
      $uuid = rand(1000000, 9999999);

      $item = array(
        'action' => strtr(mediamosa_server::get_server_url($server_upload), array('{TICKET}' => $ticket_id, '{PROGRESS_ID}' => $uuid)),
        'uploadprogress_url' => strtr($server_upload[mediamosa_server_db::URI_UPLOAD_PROGRESS], array(
          '{base_uri}' => $url,
          '{PROGRESS_ID}' => $uuid,
          '{SERVER_ID}' => $server_upload[mediamosa_server_db::NID])
        ),
        'asset_id' => $asset_id,
        'mediafile_id' => $mediafile_id,
        'ticket_id' => $ticket_id,
        'progress_id' => $uuid,
        'server_id' => $server_upload[mediamosa_server_db::NID],
      );

      // mediamosa_asset_hierarchy::set_draft('new', $asset_id);
      // Allow other modules to add data.
      drupal_alter('mediamosa_upload_ticket_create', $item);

      // Set response.
      $mediamosa->add_item($item);
    }

    // Restore SOLR queue settings.
    if (class_exists('mediamosa_solr') && method_exists('mediamosa_solr', 'forceQueue')) {
      mediamosa_solr::forceQueue(FALSE);
    }
  }
}
