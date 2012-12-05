<?php
/**
 * @file
 * AWS S3 storage class module.
 */

class mediamosa_storage_s3 extends mediamosa_storage {
  
  /**
   * Name / ID of the module.
   */
  const NAME = 's3';
  
  /**
   * Return the available S3 buckets
   */
  public function get_storage_options() {
    $options = array();
    
    libraries_load('awssdk');
    if (!class_exists('AmazonS3')) {
      drupal_set_message(t('Cannot load AmazonS3 class. Please check if awssdk is installed correctly'), 'error');
      return $options;
    }
    
    try {
      $s3 = new AmazonS3(array('key' => $this->profile->{mediamosa_storage_s3_db::KEY}, 'secret' => $this->profile->{mediamosa_storage_s3_db::SECRET}));
      foreach ($s3->get_bucket_list() as $bucket) {
        $options[self::NAME . '://' . $this->profile->{mediamosa_storage_profile_db::ID} . '/' . $bucket] = t('bucket @bucket', array('@bucket' => $bucket));
      }
    }
    catch(RequestCore_Exception $e){
      if(strstr($e->getMessage(), 'SSL certificate problem')) {
        drupal_set_message(t('There was a problem with the SSL certificate. Try setting AWS_CERTIFICATE_AUTHORITY to true in "libraries/awssdk/config.inc.php". You may also have a curl library (e.g. the default shipped with MAMP) that does not contain trust certificates for the major authorities.'), 'error');
      }
      else {
        drupal_set_message(t('There was a problem connecting to S3'), 'error');
      }
    }
    catch(Exception $e) {
      drupal_set_message(t('There was a problem retrieving the buckets from S3'));
    }
  
    return $options;
  }
  
  public function get_upload_parameters($app_id, $user_id, $group_id) {
    return array();
  }
}
