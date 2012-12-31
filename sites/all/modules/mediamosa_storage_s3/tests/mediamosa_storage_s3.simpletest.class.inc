<?php
/**
 * @file
 * MediaMosa storage S3 base class.
 */

class MediaMosaS3BaseTestCaseEga extends MediaMosaTestCaseEgaJob {
  // --------------------------------------------------------------------- Vars.
  /**
   * The amazon S3 object.
   *
   * @var MediaMosaAmazonS3
   */
  protected $s3 = NULL;

  // -------------------------------------------------------------- Constructor.
  function __construct($test_id = NULL) {
    parent::__construct($test_id);
    $this->skipClasses[__CLASS__] = TRUE;
  }

  // ------------------------------------------------------------------ Methods.
  /**
   * Implements setUp().
   */
  protected function setUp() {
    // Run parent first so we are inside sandbox.
    // Call parent::setUp and preserve arguments.
    $args = func_get_args();

    // If debug is disabled, we don't run.
    if (!variable_get('mediamosa_storage_s3_simpletest_enable', FALSE)) {
      parent::setUp();
      return;
    }

    $mediamosa_storage_s3_vars = array();
    $variables = db_select('variable', 'v')
      ->fields('v', array('name'))
      ->condition('name', db_like('mediamosa_storage_s3_') . '%', 'LIKE')
      ->execute()
      ->fetchCol();

    $this->var_export($variables);

    // Copy all variables of mediamosa_storage_s3.
    foreach ($variables as $name) {
      $mediamosa_storage_s3_vars[$name] = variable_get($name);
    }

    // Create and setup the CORE module.
    $args = array_unique(array_merge(array(
      'awssdk',
      'mediamosa_storage',
      'mediamosa_storage_s3',
    ), $args));

    if (drupal_substr(phpversion(), 0, 3) < '5.3') {
      call_user_func_array(array($this, 'parent::setUp'), $args);
    }
    else {
      call_user_func_array('parent::setUp', $args);
    }

    // Set simpletest variables in sandbox.
    foreach ($mediamosa_storage_s3_vars as $name => $value) {
      variable_set($name, $value);
    }

    $mediamosa_storage_s3_simpletest_aws_key = variable_get('mediamosa_storage_s3_simpletest_aws_key');
    $mediamosa_storage_s3_simpletest_aws_secret = variable_get('mediamosa_storage_s3_simpletest_aws_secret');
    $mediamosa_storage_s3_simpletest_aws_region = variable_get('mediamosa_storage_s3_simpletest_aws_region', MediaMosaAmazonS3::REGION_IRELAND);

    $this->assert(!empty($mediamosa_storage_s3_simpletest_aws_key), t('Simpletest login key is set'));
    $this->assert(!empty($mediamosa_storage_s3_simpletest_aws_secret), t('Simpletest login secret is set'));

    if (!empty($mediamosa_storage_s3_simpletest_aws_key) && !empty($mediamosa_storage_s3_simpletest_aws_secret)) {

      try {
        // Build s3 class.
        $this->s3 = mediamosa_storage_s3::boot($mediamosa_storage_s3_simpletest_aws_key, $mediamosa_storage_s3_simpletest_aws_secret);

        $this->assert(!empty($this->s3), 'Got s3 object');
        if ($this->s3) {
          // Export object.
          $this->var_export_verbose($this->s3);

          $this->var_export($this->s3->get_bucket_headers($this->getBucketName()));

          // Create the simpletest bucket if not already.
          if (!$this->s3->if_bucket_exists($this->getBucketName())) {
            // Create the simpletest bucket.
            $this->isOKS3Response($this->s3->create_bucket($this->getBucketName(), $mediamosa_storage_s3_simpletest_aws_region));
          }
          else {
            $this->pass('Not creating bucket, bucket exists.');
          }

          $this->assert($this->s3->if_bucket_exists($this->getBucketName()), 'Bucket exists.');
        }
      }
      catch (mediamosa_storage_s3_exception $e) {
        $this->s3 = FALSE;
      }
      catch (Exception $e) {
        $this->s3 = FALSE;
      }
    }
  }

  /**
   * Implements tearDown().
   */
  protected function tearDown() {
    // Do tear down.
    parent::tearDown();

    if ($this->s3 && $this->s3->if_bucket_exists($this->getBucketName())) {
      $this->assert($this->s3->if_bucket_exists($this->getBucketName()), 'Bucket exists.');

      // Remove our test bucket.
      $this->isOKS3Response($this->s3->delete_bucket($this->getBucketName(), TRUE));
    }
  }

  /**
   * Check the response.
   *
   * @param CFResponse $response
   *   The response.
   */
  protected function isOKS3Response(CFResponse $response) {
    if ($response === FALSE || !($response instanceof CFResponse)) {
      $this->fail('Response S3 failure; received FALSE');
      return;
    }

    $this->assert($response->isOK(), t('Response S3 OK, got code @code', array('@code' => $response->status)));
//    if (!$response->isOK()) {
      $this->var_export($response->header, 'S3Response Header');
      $this->var_export($response->body, 'S3Response Body');
  //  }
  }

  /**
   * Return the current bucket name.
   *
   * @return string
   *   The specific bucket type.
   */
  protected function getBucketName() {
    return variable_get('mediamosa_storage_s3_bucket_name', 'mediamosasimpletests3') . $this->testId;
  }

  /**
   * Create the default S3 storage profile.
   *
   * @param integer $app_id
   *   The app owner of the profile.
   *
   * @return array
   *   The storage profile.
   */
  protected function createS3Profile($app_id = 0) {
    $mediamosa_storage_s3_simpletest_aws_key = variable_get('mediamosa_storage_s3_simpletest_aws_key');
    $mediamosa_storage_s3_simpletest_aws_secret = variable_get('mediamosa_storage_s3_simpletest_aws_secret');

    // Create the one and only default local storage
    $node = mediamosa_node::create_basic_node(mediamosa_node::MEDIAMOSA_NODE_TYPE_STORAGE_PROFILE, 'MediaMosa S3 storage');
    $node->{mediamosa_storage_profile_db::TYPE} = mediamosa_storage_s3::NAME;
    $node->{mediamosa_storage_profile_db::APP_ID} = $app_id;
    $node->{mediamosa_storage_profile_db::VERSION} = 0;
    $node->{mediamosa_storage_profile_db::PROFILE} = 'MediaMosa S3 storage';
    $node->{mediamosa_storage_profile_db::IS_DEFAULT_PROFILE} = mediamosa_storage_profile_db::IS_DEFAULT_PROFILE_FALSE;
    $node->{mediamosa_storage_s3_db::KEY} = $mediamosa_storage_s3_simpletest_aws_key;
    $node->{mediamosa_storage_s3_db::SECRET} = $mediamosa_storage_s3_simpletest_aws_secret;
    node_save($node);

    $node_profile = node_load($node->nid);
    $this->var_export($node_profile);

    $storage_profile =  $this->getStorageProfile($node_profile->{mediamosa_storage_profile_db::ID});

    // Attach storage profile to my app.
    $this->attachStorageProfile($app_id ? $app_id : $this->a_app['app_id'], $storage_profile[mediamosa_storage_profile_db::ID], $this->getBucketName(), $storage_profile[mediamosa_storage_profile_db::ID], $this->getBucketName());

    return $storage_profile;
  }
}
