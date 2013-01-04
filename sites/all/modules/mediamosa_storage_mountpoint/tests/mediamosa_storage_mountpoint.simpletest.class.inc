<?php
/**
 * @file
 * MediaMosa storage mountpoint base class.
 */

class MediaMosaStorageMountPointBaseTestCaseEga extends MediaMosaTestCaseEgaJob {
  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Run parent first so we are inside sandbox.
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array(
      'mediamosa_storage',
      'mediamosa_storage_mountpoint',
    ), $args));

    if (drupal_substr(phpversion(), 0, 3) < '5.3') {
      call_user_func_array(array($this, 'parent::setUp'), $args);
    }
    else {
      call_user_func_array('parent::setUp', $args);
    }
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Create the default S3 storage profile.
   *
   * @param integer $app_id
   *   The app owner of the profile.
   *
   * @return array
   *   The storage profile.
   */
  protected function createMountPointProfile($app_id = 0) {
    // Create wrapper.
    $wrapper = new mediamosa_storage_simpletest_stream_wrapper();

    // Create the one and only default local storage
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE, 'MediaMosa mountpoint storage');
    $node->{mediamosa_storage_profile_db::TYPE} = mediamosa_storage_mountpoint::NAME;
    $node->{mediamosa_storage_profile_db::APP_ID} = $app_id;
    $node->{mediamosa_storage_profile_db::VERSION} = 0;
    $node->{mediamosa_storage_profile_db::PROFILE} = 'MediaMosa Mount Point storage';
    $node->{mediamosa_storage_profile_db::IS_DEFAULT_PROFILE} = mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_FALSE;
    $node->{mediamosa_storage_mountpoint_db::MOUNTPOINT} = $wrapper->get_storage_path_linux();
    $node->{mediamosa_storage_mountpoint_db::MOUNTPOINT_WINDOWS} = $wrapper->get_storage_path_windows();
    node_save($node);

    $node_profile = node_load($node->nid);
    $this->var_export($node_profile);

    $storage_profile = $this->getStorageProfile($node_profile->{mediamosa_storage_profile_db::ID});

    $this->var_export($storage_profile);

    // Attach storage profile to my app.
    $this->attachStorageProfile($app_id ? $app_id : $this->a_app['app_id'], $storage_profile[mediamosa_storage_profile_db::ID], 'mountpoint', $storage_profile[mediamosa_storage_profile_db::ID], 'mountpoint/stills');

    return $storage_profile;
  }
}
