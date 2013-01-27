<?php
/**
 * @file
 * AWS S3 storage class module.
 */

class mediamosa_storage_s3 extends mediamosa_storage {
  // ------------------------------------------------------------------- Consts.
  /**
   * Name / ID of the module.
   */
  const NAME = 's3';
  const SCHEME = 'mediamosa-storage-s3';

  // -------------------------------------------------------------- Constructor.
  /**
   * Main constructor
   *
   * @param array $storage_profile
   *   Storage profile.
   */
  public function __construct(array $storage_profile) {
    // Get the specific data for s3.
    $storage_s3 = mediamosa_db::db_select(mediamosa_storage_s3_db::TABLE_NAME, 's')
      ->fields('s')
      ->condition(mediamosa_storage_s3_db::PROFILE_ID, $storage_profile[mediamosa_storage_profile_db::ID])
      ->execute()
      ->fetchAssoc();

    // Merge.
    $storage_profile += $storage_s3;

    // Call parent.
    parent::__construct($storage_profile);
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Return the available S3 buckets
   */
  public function get_storage_options() {
    // Store the results.
    $options = array();

    try {
      // Boot it.
      $s3 = self::boot($this->storage_profile[mediamosa_storage_s3_db::KEY], $this->storage_profile[mediamosa_storage_s3_db::SECRET]);
      foreach ($s3->get_bucket_list() as $bucket) {
        $options[$this->storage_profile[mediamosa_storage_profile_db::ID] . '/' . $bucket] = t('bucket @bucket', array('@bucket' => $bucket));
      }
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
    }

    return $options;
  }

  /**
   * Create the s3 object.
   *
   * @return MediaMosaAmazonS3
   *   The amazon object.
   */
  public function create_s3() {
    return self::boot($this->storage_profile[mediamosa_storage_s3_db::KEY], $this->storage_profile[mediamosa_storage_s3_db::SECRET]);
  }

  public function get_upload_parameters($app_id, $user_id, $group_id) {
    return array();
  }

  /**
   * Get the default scheme.
   *
   * @param boolean $is_still
   *   Is mount point for still? Does not apply for external locations.
   *
   * @return string
   *   The uri of the moint point to store.
   */
  public function get_scheme($is_still = FALSE) {
    return self::SCHEME;
  }

  // ---------------------------------------------------------------- Functions.
  /**
   * Get the S3 regions.
   *
   * @return array
   *   The S3 regions.
   */
  public static function get_s3_regions() {
    // Load libraries.
    self::load_libraries();

    return array(
      AmazonS3::REGION_US_E1 => 'US Standard',
      AmazonS3::REGION_US_W1 => 'Northern California',
      AmazonS3::REGION_US_W2 => 'Oregon',
      AmazonS3::REGION_EU_W1 => 'Ireland',
      AmazonS3::REGION_APAC_SE1 => 'Singapore',
      AmazonS3::REGION_APAC_NE1 => 'Tokyo',
      AmazonS3::REGION_SA_E1 => 'Sao Paulo',
    );
  }

  /**
   * Load the required libraries.
   *
   * @throws mediamosa_storage_s3_exception
   */
  public static function load_libraries() {
    if (!libraries_load('awssdk')) {
      throw new mediamosa_storage_s3_exception('Unable to load the AWS SDK. Please check if awssdk is installed correctly.');
    }
  }

  /**
   * Startup the storage status.
   *
   * @param string $key
   *   Your AWS key, or a session key.
   * @param string $secret
   *   Your AWS secret key, or a session secret key.
   *
   * @return MediaMosaAmazonS3
   *   The MediaMosa AmazonS3 object.
   *
   * @throws mediamosa_storage_s3_exception
   */
  public static function boot($key, $secret) {
    // Basic tests.

    // Load libraries.
    self::load_libraries();

    if (!class_exists('AmazonS3')) {
      throw new mediamosa_storage_s3_exception('Cannot load AmazonS3 class. Please check if awssdk is installed correctly');
    }

    // Test the AWS connection
    try {
      $s3 = new MediaMosaAmazonS3(array('key' => $key, 'secret' => $secret));
      $user_id = $s3->get_canonical_user_id();
      if (!$user_id['id']) {
        throw new mediamosa_storage_s3_exception('The S3 access credentials are invalid');
      }
    }
    catch (RequestCore_Exception $e){
      if (strstr($e->getMessage(), 'SSL certificate problem')) {
        throw new mediamosa_storage_s3_exception('There was a problem with the SSL certificate. Try setting AWS_CERTIFICATE_AUTHORITY to true in "libraries/awssdk/config.inc.php". You may also have a curl library (e.g. the default shipped with MAMP) that does not contain trust certificates for the major authorities.');
      }

      throw new mediamosa_storage_s3_exception('There was a problem connecting to S3');
    }
    catch(Exception $e) {
      throw new mediamosa_storage_s3_exception(t("There was a problem connecting to S3, message: '@message'", array('@message' => $e->getMessage())));
    }

    return $s3;
  }

  /**
   * Do a login test.
   *
   * @param string $key
   *   Your AWS key, or a session key.
   * @param string $secret
   *   Your AWS secret key, or a session secret key.
   *
   * @return boolean
   *   Returns TRUE when succesful, FALSE otherwise.
   */
  public static function login_test($key, $secret) {
    try {
      // Do startup.
      self::boot($key, $secret);
    }
    catch (Exception $e) {
      return FALSE;
    }

    return TRUE;
  }

  /**
   * Test if the mount point points to S3.
   *
   * @param string $mount_point
   *   The mount point to check.
   *
   * @return boolean
   *   Returns TRUE when hosted on S3.
   */
  public static function mediafile_is_s3($mount_point) {
    return mediamosa_unicode::substr($mount_point, 0, strlen(self::SCHEME)) == self::SCHEME;
  }

  /**
   * Test if the mediafile is public or private.
   *
   * @param array $mediafile
   *   The mediafile to test.
   *
   * @return boolean
   *   Returns TRUE when public.
   *
   */
  public static function mediafile_is_public($mediafile) {
    try {
      return mediamosa_media::is_public($mediafile, FALSE);
    }
    catch(mediamosa_exception $e) {
      return FALSE;
    }
  }
}
