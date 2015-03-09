<?php
/**
 * @file
 * mediamosa_feature.features.mediamosa_transcode_profile.inc
 */

/**
 * Implements hook_mediamosa_transcode_profiles().
 */
function mediamosa_feature_mediamosa_transcode_profiles() {
  $mediamosa_transcode_profiles = array();

  // Exported mediamosa transcode profile: mp4 h264 360p.
  $mediamosa_transcode_profiles['mp4_h264_360p'] = array(
    'app_id' => 0,
    'version' => 0,
    'profile' => 'mp4 h264 360p',
    'is_default_profile' => 'FALSE',
    'tool' => 'ffmpeg',
    'file_extension' => 'mp4',
    'command' => 'videocodec=libx264&videobitrate=500000&fps=30&size=640x360&maintain_aspect_ratio=yes&audiocodec=aac&strict=experimentalaudiobitrate%3A128000&audiosamplingrate=44100&audiochannels=2',
    'machine_name' => 'mp4_h264_360p',
  );
  // Exported mediamosa transcode profile: mp4 h264 360p (mobile).
  $mediamosa_transcode_profiles['mp4_h264_360p_mobile'] = array(
    'app_id' => 0,
    'version' => 0,
    'profile' => 'mp4 h264 360p (mobile)',
    'is_default_profile' => 'FALSE',
    'tool' => 'ffmpeg',
    'file_extension' => 'mp4',
    'command' => 'videocodec=libx264&x264_profile=baseline&x264_preset=slow&videobitrate=500000&fps=30&size=640x360&maintain_aspect_ratio=yes&audiocodec=aac&strict=experimentalaudiobitrate%3A128000&audiosamplingrate=44100&audiochannels=2',
    'machine_name' => 'mp4_h264_360p_mobile',
  );
  // Exported mediamosa transcode profile: mp4 h264 480p.
  $mediamosa_transcode_profiles['mp4_h264_480p'] = array(
    'app_id' => 0,
    'version' => 0,
    'profile' => 'mp4 h264 480p',
    'is_default_profile' => 'FALSE',
    'tool' => 'ffmpeg',
    'file_extension' => 'mp4',
    'command' => 'videocodec=libx264&videobitrate=1000000&fps=30&size=854x480&maintain_aspect_ratio=yes&audiocodec=aac&strict=experimentalaudiobitrate%3A128000&audiosamplingrate=44100&audiochannels=2',
    'machine_name' => 'mp4_h264_480p',
  );
  // Exported mediamosa transcode profile: mp4 h264 480p (mobile).
  $mediamosa_transcode_profiles['mp4_h264_480p_mobile'] = array(
    'app_id' => 0,
    'version' => 0,
    'profile' => 'mp4 h264 480p (mobile)',
    'is_default_profile' => 'FALSE',
    'tool' => 'ffmpeg',
    'file_extension' => 'mp4',
    'command' => 'videocodec=libx264&x264_profile=baseline&x264_preset=slow&videobitrate=1000000&fps=30&size=854x480&maintain_aspect_ratio=yes&audiocodec=aac&strict=experimentalaudiobitrate%3A128000&audiosamplingrate=44100&audiochannels=2',
    'machine_name' => 'mp4_h264_480p_mobile',
  );
  // Exported mediamosa transcode profile: mp4 h264 720p.
  $mediamosa_transcode_profiles['mp4_h264_720p'] = array(
    'app_id' => 0,
    'version' => 0,
    'profile' => 'mp4 h264 720p',
    'is_default_profile' => 'FALSE',
    'tool' => 'ffmpeg',
    'file_extension' => 'mp4',
    'command' => 'videocodec=libx264&videobitrate=2000000&fps=30&size=1280x720&maintain_aspect_ratio=yes&audiocodec=aac&strict=experimentalaudiobitrate%3A192000&audiosamplingrate=44100&audiochannels=2',
    'machine_name' => 'mp4_h264_720p',
  );
  // Exported mediamosa transcode profile: mp4 h264 720p (mobile).
  $mediamosa_transcode_profiles['mp4_h264_720p_mobile'] = array(
    'app_id' => 0,
    'version' => 0,
    'profile' => 'mp4 h264 720p (mobile)',
    'is_default_profile' => 'FALSE',
    'tool' => 'ffmpeg',
    'file_extension' => 'mp4',
    'command' => 'videocodec=libx264&x264_profile=baseline&x264_preset=slow&videobitrate=2000000&fps=30&size=1280x720&maintain_aspect_ratio=yes&audiocodec=aac&strict=experimentalaudiobitrate%3A192000&audiosamplingrate=44100&audiochannels=2',
    'machine_name' => 'mp4_h264_720p_mobile',
  );

  return $mediamosa_transcode_profiles;
}
