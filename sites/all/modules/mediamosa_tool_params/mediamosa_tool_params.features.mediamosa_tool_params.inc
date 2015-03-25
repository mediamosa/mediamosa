<?php
/**
 * @file
 * mediamosa_tool_params.features.mediamosa_tool_params.inc
 */

/**
 * Implements hook_mediamosa_tool_params().
 */
function mediamosa_tool_params_mediamosa_tool_params() {
  $mediamosa_tool_params = array();

  // Exported mediamosa tool ffmpeg params: internal_previous_job.
  $mediamosa_tool_params['ffmpeg_internal_previous_job'] = array(
    'tool' => 'ffmpeg',
    'weight' => 0,
    'nice_parameter' => 'internal_previous_job',
    'tool_parameter' => '',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => NULL,
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => NULL,
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_internal_previous_job',
  );
  // Exported mediamosa tool ffmpeg params: internal_file_extension.
  $mediamosa_tool_params['ffmpeg_internal_file_extension'] = array(
    'tool' => 'ffmpeg',
    'weight' => 1,
    'nice_parameter' => 'internal_file_extension',
    'tool_parameter' => '',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:55:{i:0;s:9:"a:27:{i:0";i:1;s:15:"s:9:"a:13:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:6:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:22:"s:15:"s:9:"s:3:"flv"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:22:"s:15:"s:9:"s:3:"wmv"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:22:"s:15:"s:9:"s:3:"mp4"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:22:"s:15:"s:9:"s:3:"avi"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:22:"s:15:"s:9:"s:3:"ogg"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:24:"s:17:"s:10:"s:4:"webm"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:13:"s:7:"s:1:"}""";i:52;s:4:"i:26";i:53;s:7:"s:1:"}"";i:54;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => NULL,
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_internal_file_extension',
  );
  // Exported mediamosa tool ffmpeg params: strictt.
  $mediamosa_tool_params['ffmpeg_strictt'] = array(
    'tool' => 'ffmpeg',
    'weight' => 2,
    'nice_parameter' => 'strictt',
    'tool_parameter' => '-strict',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:47:{i:0;s:9:"a:23:{i:0";i:1;s:15:"s:9:"a:11:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:5:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:28:"s:21:"s:14:"s:8:"a:2:{i:0"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:26:"s:19:"s:12:"s:6:"s:0:"""""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:22:"s:15:"s:9:"s:3:"i:1"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:40:"s:33:"s:26:"s:19:"s:12:"experimental""""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:20:"s:13:"s:7:"s:1:"}"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:13:"s:7:"s:1:"}""";i:44;s:4:"i:22";i:45;s:7:"s:1:"}"";i:46;s:1:"}";}',
    'default_value' => '',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'allow non standardized experimental things.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_strictt',
  );
  // Exported mediamosa tool ffmpeg params: videocodec.
  $mediamosa_tool_params['ffmpeg_videocodec'] = array(
    'tool' => 'ffmpeg',
    'weight' => 3,
    'nice_parameter' => 'videocodec',
    'tool_parameter' => '-vcodec',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:71:{i:0;s:9:"a:35:{i:0";i:1;s:15:"s:9:"a:17:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:8:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:27:"s:20:"s:13:"s:7:"libx264"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:29:"s:22:"s:15:"s:9:"libtheora"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:26:"s:19:"s:12:"s:6:"libvpx"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:28:"s:21:"s:14:"s:8:"rawvideo"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:22:"s:15:"s:9:"s:3:"flv"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:22:"s:15:"s:9:"s:3:"wmv"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:15:"s:9:"s:3:"i:6""";i:52;s:4:"i:26";i:53;s:10:"s:4:"i:13"";i:54;s:4:"i:27";i:55;s:24:"s:17:"s:10:"s:4:"h263"""";i:56;s:4:"i:28";i:57;s:10:"s:4:"i:14"";i:58;s:4:"i:29";i:59;s:15:"s:9:"s:3:"i:7""";i:60;s:4:"i:30";i:61;s:10:"s:4:"i:15"";i:62;s:4:"i:31";i:63;s:25:"s:18:"s:11:"s:5:"mpeg4"""";i:64;s:4:"i:32";i:65;s:10:"s:4:"i:16"";i:66;s:4:"i:33";i:67;s:13:"s:7:"s:1:"}""";i:68;s:4:"i:34";i:69;s:7:"s:1:"}"";i:70;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Force video codec to codec. Use the copy special value to tell that the raw codec data must be copied as is. If libx264 (h264) is used, be sure to use libfaac audiocodec@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_videocodec',
  );
  // Exported mediamosa tool ffmpeg params: videopreset_quality.
  $mediamosa_tool_params['ffmpeg_videopreset_quality'] = array(
    'tool' => 'ffmpeg',
    'weight' => 4,
    'nice_parameter' => 'videopreset_quality',
    'tool_parameter' => '-vpre',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:63:{i:0;s:9:"a:31:{i:0";i:1;s:15:"s:9:"a:15:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:7:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:28:"s:21:"s:14:"s:8:"veryslow"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:26:"s:19:"s:12:"s:6:"slower"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:24:"s:17:"s:10:"s:4:"slow"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:24:"s:17:"s:10:"s:4:"fast"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:26:"s:19:"s:12:"s:6:"faster"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:15:"s:9:"s:3:"i:6""";i:52;s:4:"i:26";i:53;s:10:"s:4:"i:13"";i:54;s:4:"i:27";i:55;s:28:"s:21:"s:14:"s:8:"veryfast"""";i:56;s:4:"i:28";i:57;s:10:"s:4:"i:14"";i:58;s:4:"i:29";i:59;s:13:"s:7:"s:1:"}""";i:60;s:4:"i:30";i:61;s:7:"s:1:"}"";i:62;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'The -vpre option is an preset option of ffmpeg used till version 0.8 for libx264 and later used only for libvpx, it is recommended that you use -profile and -preset for libx264.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_videopreset_quality',
  );
  // Exported mediamosa tool ffmpeg params: videopreset_profile.
  $mediamosa_tool_params['ffmpeg_videopreset_profile'] = array(
    'tool' => 'ffmpeg',
    'weight' => 5,
    'nice_parameter' => 'videopreset_profile',
    'tool_parameter' => '-vpre',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:47:{i:0;s:9:"a:23:{i:0";i:1;s:15:"s:9:"a:11:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:5:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:24:"s:17:"s:10:"s:4:"main"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:28:"s:21:"s:14:"s:8:"baseline"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:27:"s:20:"s:13:"s:7:"ipod320"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:27:"s:20:"s:13:"s:7:"ipod640"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:13:"s:7:"s:1:"}""";i:44;s:4:"i:22";i:45;s:7:"s:1:"}"";i:46;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => NULL,
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_videopreset_profile',
  );
  // Exported mediamosa tool ffmpeg params: x264_profile.
  $mediamosa_tool_params['ffmpeg_x264_profile'] = array(
    'tool' => 'ffmpeg',
    'weight' => 6,
    'nice_parameter' => 'x264_profile',
    'tool_parameter' => '-profile',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:39:{i:0;s:9:"a:19:{i:0";i:1;s:14:"s:8:"a:9:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:4:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:28:"s:21:"s:14:"s:8:"baseline"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:24:"s:17:"s:10:"s:4:"main"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:24:"s:17:"s:10:"s:4:"high"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:13:"s:7:"s:1:"}""";i:36;s:4:"i:18";i:37;s:7:"s:1:"}"";i:38;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'see http://mewiki.project357.com/wiki/X264_Settings for more information.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_x264_profile',
  );
  // Exported mediamosa tool ffmpeg params: x264_preset.
  $mediamosa_tool_params['ffmpeg_x264_preset'] = array(
    'tool' => 'ffmpeg',
    'weight' => 7,
    'nice_parameter' => 'x264_preset',
    'tool_parameter' => '-preset',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:95:{i:0;s:9:"a:47:{i:0";i:1;s:15:"s:9:"a:23:{i:0"";i:2;s:3:"i:1";i:3;s:22:"s:15:"s:9:"a:11:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:29:"s:22:"s:15:"s:9:"ultrafast"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:29:"s:22:"s:15:"s:9:"superfast"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:28:"s:21:"s:14:"s:8:"veryfast"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:26:"s:19:"s:12:"s:6:"faster"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:24:"s:17:"s:10:"s:4:"fast"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:15:"s:9:"s:3:"i:6""";i:52;s:4:"i:26";i:53;s:10:"s:4:"i:13"";i:54;s:4:"i:27";i:55;s:26:"s:19:"s:12:"s:6:"medium"""";i:56;s:4:"i:28";i:57;s:10:"s:4:"i:14"";i:58;s:4:"i:29";i:59;s:15:"s:9:"s:3:"i:7""";i:60;s:4:"i:30";i:61;s:10:"s:4:"i:15"";i:62;s:4:"i:31";i:63;s:24:"s:17:"s:10:"s:4:"slow"""";i:64;s:4:"i:32";i:65;s:10:"s:4:"i:16"";i:66;s:4:"i:33";i:67;s:15:"s:9:"s:3:"i:8""";i:68;s:4:"i:34";i:69;s:10:"s:4:"i:17"";i:70;s:4:"i:35";i:71;s:26:"s:19:"s:12:"s:6:"slower"""";i:72;s:4:"i:36";i:73;s:10:"s:4:"i:18"";i:74;s:4:"i:37";i:75;s:15:"s:9:"s:3:"i:9""";i:76;s:4:"i:38";i:77;s:10:"s:4:"i:19"";i:78;s:4:"i:39";i:79;s:28:"s:21:"s:14:"s:8:"veryslow"""";i:80;s:4:"i:40";i:81;s:10:"s:4:"i:20"";i:82;s:4:"i:41";i:83;s:17:"s:10:"s:4:"i:10""";i:84;s:4:"i:42";i:85;s:10:"s:4:"i:21"";i:86;s:4:"i:43";i:87;s:27:"s:20:"s:13:"s:7:"placebo"""";i:88;s:4:"i:44";i:89;s:10:"s:4:"i:22"";i:90;s:4:"i:45";i:91;s:13:"s:7:"s:1:"}""";i:92;s:4:"i:46";i:93;s:7:"s:1:"}"";i:94;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'see http://mewiki.project357.com/wiki/X264_Settings for more information.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_x264_preset',
  );
  // Exported mediamosa tool ffmpeg params: 2_pass_h264_encoding.
  $mediamosa_tool_params['ffmpeg_2_pass_h264_encoding'] = array(
    'tool' => 'ffmpeg',
    'weight' => 8,
    'nice_parameter' => '2_pass_h264_encoding',
    'tool_parameter' => '-pass',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:31:{i:0;s:9:"a:15:{i:0";i:1;s:14:"s:8:"a:7:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:3:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:20:"s:13:"s:7:"s:1:"1"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:20:"s:13:"s:7:"s:1:"2"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:13:"s:7:"s:1:"}""";i:28;s:4:"i:14";i:29;s:7:"s:1:"}"";i:30;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Support 2 pass encoding for better h264 quality (pass=2), corresponding the ffmpeg-h264 2 pass encoding',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_2_pass_h264_encoding',
  );
  // Exported mediamosa tool ffmpeg params: videobitrate.
  $mediamosa_tool_params['ffmpeg_videobitrate'] = array(
    'tool' => 'ffmpeg',
    'weight' => 9,
    'nice_parameter' => 'videobitrate',
    'tool_parameter' => '-b',
    'min_value' => 56000,
    'max_value' => 80000000,
    'allowed_value' => NULL,
    'default_value' => 200000,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Set the video bitrate in bit/s@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_videobitrate',
  );
  // Exported mediamosa tool ffmpeg params: fps.
  $mediamosa_tool_params['ffmpeg_fps'] = array(
    'tool' => 'ffmpeg',
    'weight' => 10,
    'nice_parameter' => 'fps',
    'tool_parameter' => '-r',
    'min_value' => 10,
    'max_value' => 30,
    'allowed_value' => NULL,
    'default_value' => 25,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Set frame rate (Hz value, fraction or abbereviation@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_fps',
  );
  // Exported mediamosa tool ffmpeg params: qmax.
  $mediamosa_tool_params['ffmpeg_qmax'] = array(
    'tool' => 'ffmpeg',
    'weight' => 11,
    'nice_parameter' => 'qmax',
    'tool_parameter' => '-qmax',
    'min_value' => 1,
    'max_value' => 31,
    'allowed_value' => NULL,
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Maximum video quantizer scale (VBR)@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_qmax',
  );
  // Exported mediamosa tool ffmpeg params: duration.
  $mediamosa_tool_params['ffmpeg_duration'] = array(
    'tool' => 'ffmpeg',
    'weight' => 12,
    'nice_parameter' => 'duration',
    'tool_parameter' => '-t',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => NULL,
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Duration of the video.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_duration',
  );
  // Exported mediamosa tool ffmpeg params: startposition.
  $mediamosa_tool_params['ffmpeg_startposition'] = array(
    'tool' => 'ffmpeg',
    'weight' => 13,
    'nice_parameter' => 'startposition',
    'tool_parameter' => '-ss',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => NULL,
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => NULL,
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_startposition',
  );
  // Exported mediamosa tool ffmpeg params: size.
  $mediamosa_tool_params['ffmpeg_size'] = array(
    'tool' => 'ffmpeg',
    'weight' => 14,
    'nice_parameter' => 'size',
    'tool_parameter' => '-s',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:287:{i:0;s:10:"a:143:{i:0";i:1;s:15:"s:9:"a:71:{i:0"";i:2;s:3:"i:1";i:3;s:22:"s:15:"s:9:"a:35:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:26:"s:19:"s:12:"s:6:"128x96"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:27:"s:20:"s:13:"s:7:"176x144"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:27:"s:20:"s:13:"s:7:"352x288"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:27:"s:20:"s:13:"s:7:"704x576"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:27:"s:20:"s:13:"s:7:"160x120"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:15:"s:9:"s:3:"i:6""";i:52;s:4:"i:26";i:53;s:10:"s:4:"i:13"";i:54;s:4:"i:27";i:55;s:27:"s:20:"s:13:"s:7:"320x240"""";i:56;s:4:"i:28";i:57;s:10:"s:4:"i:14"";i:58;s:4:"i:29";i:59;s:15:"s:9:"s:3:"i:7""";i:60;s:4:"i:30";i:61;s:10:"s:4:"i:15"";i:62;s:4:"i:31";i:63;s:27:"s:20:"s:13:"s:7:"640x480"""";i:64;s:4:"i:32";i:65;s:10:"s:4:"i:16"";i:66;s:4:"i:33";i:67;s:15:"s:9:"s:3:"i:8""";i:68;s:4:"i:34";i:69;s:10:"s:4:"i:17"";i:70;s:4:"i:35";i:71;s:27:"s:20:"s:13:"s:7:"800x600"""";i:72;s:4:"i:36";i:73;s:10:"s:4:"i:18"";i:74;s:4:"i:37";i:75;s:15:"s:9:"s:3:"i:9""";i:76;s:4:"i:38";i:77;s:10:"s:4:"i:19"";i:78;s:4:"i:39";i:79;s:28:"s:21:"s:14:"s:8:"1024x768"""";i:80;s:4:"i:40";i:81;s:10:"s:4:"i:20"";i:82;s:4:"i:41";i:83;s:17:"s:10:"s:4:"i:10""";i:84;s:4:"i:42";i:85;s:10:"s:4:"i:21"";i:86;s:4:"i:43";i:87;s:29:"s:22:"s:15:"s:9:"1600x1200"""";i:88;s:4:"i:44";i:89;s:10:"s:4:"i:22"";i:90;s:4:"i:45";i:91;s:17:"s:10:"s:4:"i:11""";i:92;s:4:"i:46";i:93;s:10:"s:4:"i:23"";i:94;s:4:"i:47";i:95;s:29:"s:22:"s:15:"s:9:"2048x1536"""";i:96;s:4:"i:48";i:97;s:10:"s:4:"i:24"";i:98;s:4:"i:49";i:99;s:17:"s:10:"s:4:"i:12""";i:100;s:4:"i:50";i:101;s:10:"s:4:"i:25"";i:102;s:4:"i:51";i:103;s:29:"s:22:"s:15:"s:9:"1280x1024"""";i:104;s:4:"i:52";i:105;s:10:"s:4:"i:26"";i:106;s:4:"i:53";i:107;s:17:"s:10:"s:4:"i:13""";i:108;s:4:"i:54";i:109;s:10:"s:4:"i:27"";i:110;s:4:"i:55";i:111;s:29:"s:22:"s:15:"s:9:"2560x2048"""";i:112;s:4:"i:56";i:113;s:10:"s:4:"i:28"";i:114;s:4:"i:57";i:115;s:17:"s:10:"s:4:"i:14""";i:116;s:4:"i:58";i:117;s:10:"s:4:"i:29"";i:118;s:4:"i:59";i:119;s:29:"s:22:"s:15:"s:9:"5120x4096"""";i:120;s:4:"i:60";i:121;s:10:"s:4:"i:30"";i:122;s:4:"i:61";i:123;s:17:"s:10:"s:4:"i:15""";i:124;s:4:"i:62";i:125;s:10:"s:4:"i:31"";i:126;s:4:"i:63";i:127;s:27:"s:20:"s:13:"s:7:"852x480"""";i:128;s:4:"i:64";i:129;s:10:"s:4:"i:32"";i:130;s:4:"i:65";i:131;s:17:"s:10:"s:4:"i:16""";i:132;s:4:"i:66";i:133;s:10:"s:4:"i:33"";i:134;s:4:"i:67";i:135;s:28:"s:21:"s:14:"s:8:"1366x768"""";i:136;s:4:"i:68";i:137;s:10:"s:4:"i:34"";i:138;s:4:"i:69";i:139;s:17:"s:10:"s:4:"i:17""";i:140;s:4:"i:70";i:141;s:10:"s:4:"i:35"";i:142;s:4:"i:71";i:143;s:29:"s:22:"s:15:"s:9:"1600x1024"""";i:144;s:4:"i:72";i:145;s:10:"s:4:"i:36"";i:146;s:4:"i:73";i:147;s:17:"s:10:"s:4:"i:18""";i:148;s:4:"i:74";i:149;s:10:"s:4:"i:37"";i:150;s:4:"i:75";i:151;s:29:"s:22:"s:15:"s:9:"1920x1200"""";i:152;s:4:"i:76";i:153;s:10:"s:4:"i:38"";i:154;s:4:"i:77";i:155;s:17:"s:10:"s:4:"i:19""";i:156;s:4:"i:78";i:157;s:10:"s:4:"i:39"";i:158;s:4:"i:79";i:159;s:29:"s:22:"s:15:"s:9:"2560x1600"""";i:160;s:4:"i:80";i:161;s:10:"s:4:"i:40"";i:162;s:4:"i:81";i:163;s:17:"s:10:"s:4:"i:20""";i:164;s:4:"i:82";i:165;s:10:"s:4:"i:41"";i:166;s:4:"i:83";i:167;s:29:"s:22:"s:15:"s:9:"3200x2048"""";i:168;s:4:"i:84";i:169;s:10:"s:4:"i:42"";i:170;s:4:"i:85";i:171;s:17:"s:10:"s:4:"i:21""";i:172;s:4:"i:86";i:173;s:10:"s:4:"i:43"";i:174;s:4:"i:87";i:175;s:29:"s:22:"s:15:"s:9:"3840x2400"""";i:176;s:4:"i:88";i:177;s:10:"s:4:"i:44"";i:178;s:4:"i:89";i:179;s:17:"s:10:"s:4:"i:22""";i:180;s:4:"i:90";i:181;s:10:"s:4:"i:45"";i:182;s:4:"i:91";i:183;s:29:"s:22:"s:15:"s:9:"6400x4096"""";i:184;s:4:"i:92";i:185;s:10:"s:4:"i:46"";i:186;s:4:"i:93";i:187;s:17:"s:10:"s:4:"i:23""";i:188;s:4:"i:94";i:189;s:10:"s:4:"i:47"";i:190;s:4:"i:95";i:191;s:29:"s:22:"s:15:"s:9:"7680x4800"""";i:192;s:4:"i:96";i:193;s:10:"s:4:"i:48"";i:194;s:4:"i:97";i:195;s:17:"s:10:"s:4:"i:24""";i:196;s:4:"i:98";i:197;s:10:"s:4:"i:49"";i:198;s:4:"i:99";i:199;s:27:"s:20:"s:13:"s:7:"320x200"""";i:200;s:5:"i:100";i:201;s:10:"s:4:"i:50"";i:202;s:5:"i:101";i:203;s:17:"s:10:"s:4:"i:25""";i:204;s:5:"i:102";i:205;s:10:"s:4:"i:51"";i:206;s:5:"i:103";i:207;s:27:"s:20:"s:13:"s:7:"640x350"""";i:208;s:5:"i:104";i:209;s:10:"s:4:"i:52"";i:210;s:5:"i:105";i:211;s:17:"s:10:"s:4:"i:26""";i:212;s:5:"i:106";i:213;s:10:"s:4:"i:53"";i:214;s:5:"i:107";i:215;s:27:"s:20:"s:13:"s:7:"852x480"""";i:216;s:5:"i:108";i:217;s:10:"s:4:"i:54"";i:218;s:5:"i:109";i:219;s:17:"s:10:"s:4:"i:27""";i:220;s:5:"i:110";i:221;s:10:"s:4:"i:55"";i:222;s:5:"i:111";i:223;s:28:"s:21:"s:14:"s:8:"1280x720"""";i:224;s:5:"i:112";i:225;s:10:"s:4:"i:56"";i:226;s:5:"i:113";i:227;s:17:"s:10:"s:4:"i:28""";i:228;s:5:"i:114";i:229;s:10:"s:4:"i:57"";i:230;s:5:"i:115";i:231;s:29:"s:22:"s:15:"s:9:"1920x1080"""";i:232;s:5:"i:116";i:233;s:10:"s:4:"i:58"";i:234;s:5:"i:117";i:235;s:17:"s:10:"s:4:"i:29""";i:236;s:5:"i:118";i:237;s:10:"s:4:"i:59"";i:238;s:5:"i:119";i:239;s:27:"s:20:"s:13:"s:7:"480x320"""";i:240;s:5:"i:120";i:241;s:10:"s:4:"i:60"";i:242;s:5:"i:121";i:243;s:17:"s:10:"s:4:"i:30""";i:244;s:5:"i:122";i:245;s:10:"s:4:"i:61"";i:246;s:5:"i:123";i:247;s:27:"s:20:"s:13:"s:7:"480x270"""";i:248;s:5:"i:124";i:249;s:10:"s:4:"i:62"";i:250;s:5:"i:125";i:251;s:17:"s:10:"s:4:"i:31""";i:252;s:5:"i:126";i:253;s:10:"s:4:"i:63"";i:254;s:5:"i:127";i:255;s:27:"s:20:"s:13:"s:7:"480x360"""";i:256;s:5:"i:128";i:257;s:10:"s:4:"i:64"";i:258;s:5:"i:129";i:259;s:17:"s:10:"s:4:"i:32""";i:260;s:5:"i:130";i:261;s:10:"s:4:"i:65"";i:262;s:5:"i:131";i:263;s:27:"s:20:"s:13:"s:7:"640x390"""";i:264;s:5:"i:132";i:265;s:10:"s:4:"i:66"";i:266;s:5:"i:133";i:267;s:17:"s:10:"s:4:"i:33""";i:268;s:5:"i:134";i:269;s:10:"s:4:"i:67"";i:270;s:5:"i:135";i:271;s:27:"s:20:"s:13:"s:7:"640x360"""";i:272;s:5:"i:136";i:273;s:10:"s:4:"i:68"";i:274;s:5:"i:137";i:275;s:17:"s:10:"s:4:"i:34""";i:276;s:5:"i:138";i:277;s:10:"s:4:"i:69"";i:278;s:5:"i:139";i:279;s:27:"s:20:"s:13:"s:7:"854x480"""";i:280;s:5:"i:140";i:281;s:10:"s:4:"i:70"";i:282;s:5:"i:141";i:283;s:13:"s:7:"s:1:"}""";i:284;s:5:"i:142";i:285;s:7:"s:1:"}"";i:286;s:1:"}";}',
    'default_value' => '640x390',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Set frame size using format WIDTHxHEIGHT@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_size',
  );
  // Exported mediamosa tool ffmpeg params: aspect.
  $mediamosa_tool_params['ffmpeg_aspect'] = array(
    'tool' => 'ffmpeg',
    'weight' => 15,
    'nice_parameter' => 'aspect',
    'tool_parameter' => '-aspect',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:39:{i:0;s:9:"a:19:{i:0";i:1;s:14:"s:8:"a:9:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:4:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:19:"s:12:"s:6:"s:0:""""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:22:"s:15:"s:9:"s:3:"4:3"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:22:"s:15:"s:9:"s:3:"3:2"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:24:"s:17:"s:10:"s:4:"16:9"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:13:"s:7:"s:1:"}""";i:36;s:4:"i:18";i:37;s:7:"s:1:"}"";i:38;s:1:"}";}',
    'default_value' => '',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Aspect ratio',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_aspect',
  );
  // Exported mediamosa tool ffmpeg params: maintain_aspect_ratio.
  $mediamosa_tool_params['ffmpeg_maintain_aspect_ratio'] = array(
    'tool' => 'ffmpeg',
    'weight' => 16,
    'nice_parameter' => 'maintain_aspect_ratio',
    'tool_parameter' => '',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:23:{i:0;s:9:"a:11:{i:0";i:1;s:14:"s:8:"a:5:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:2:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:22:"s:15:"s:9:"s:3:"yes"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:21:"s:14:"s:8:"s:2:"no"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:13:"s:7:"s:1:"}""";i:20;s:4:"i:10";i:21;s:7:"s:1:"}"";i:22;s:1:"}";}',
    'default_value' => 'yes',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Force maintain the original video aspect ratio or not@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_maintain_aspect_ratio',
  );
  // Exported mediamosa tool ffmpeg params: padding.
  $mediamosa_tool_params['ffmpeg_padding'] = array(
    'tool' => 'ffmpeg',
    'weight' => 17,
    'nice_parameter' => 'padding',
    'tool_parameter' => '',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:23:{i:0;s:9:"a:11:{i:0";i:1;s:14:"s:8:"a:5:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:2:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:22:"s:15:"s:9:"s:3:"yes"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:21:"s:14:"s:8:"s:2:"no"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:13:"s:7:"s:1:"}""";i:20;s:4:"i:10";i:21;s:7:"s:1:"}"";i:22;s:1:"}";}',
    'default_value' => 'yes',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Force padding, if aspect ratio is maintained@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_padding',
  );
  // Exported mediamosa tool ffmpeg params: audiocodec.
  $mediamosa_tool_params['ffmpeg_audiocodec'] = array(
    'tool' => 'ffmpeg',
    'weight' => 18,
    'nice_parameter' => 'audiocodec',
    'tool_parameter' => '-acodec',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:79:{i:0;s:9:"a:39:{i:0";i:1;s:15:"s:9:"a:19:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:9:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:31:"s:24:"s:17:"s:10:"libmp3lame"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:22:"s:15:"s:9:"s:3:"mp3"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:29:"s:22:"s:15:"s:9:"pcm_s16le"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:22:"s:15:"s:9:"s:3:"aac"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:27:"s:20:"s:13:"s:7:"libfaac"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:33:"s:26:"s:19:"s:12:"libvo_aacenc"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:15:"s:9:"s:3:"i:6""";i:52;s:4:"i:26";i:53;s:10:"s:4:"i:13"";i:54;s:4:"i:27";i:55;s:31:"s:24:"s:17:"s:10:"libfdk_acc"""";i:56;s:4:"i:28";i:57;s:10:"s:4:"i:14"";i:58;s:4:"i:29";i:59;s:15:"s:9:"s:3:"i:7""";i:60;s:4:"i:30";i:61;s:10:"s:4:"i:15"";i:62;s:4:"i:31";i:63;s:29:"s:22:"s:15:"s:9:"libvorbis"""";i:64;s:4:"i:32";i:65;s:10:"s:4:"i:16"";i:66;s:4:"i:33";i:67;s:15:"s:9:"s:3:"i:8""";i:68;s:4:"i:34";i:69;s:10:"s:4:"i:17"";i:70;s:4:"i:35";i:71;s:26:"s:19:"s:12:"s:6:"vorbis"""";i:72;s:4:"i:36";i:73;s:10:"s:4:"i:18"";i:74;s:4:"i:37";i:75;s:13:"s:7:"s:1:"}""";i:76;s:4:"i:38";i:77;s:7:"s:1:"}"";i:78;s:1:"}";}',
    'default_value' => NULL,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Force audio codec to codec. Use the copy special value to specify that the raw codec data must be copied as is@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_audiocodec',
  );
  // Exported mediamosa tool ffmpeg params: audiobitrate.
  $mediamosa_tool_params['ffmpeg_audiobitrate'] = array(
    'tool' => 'ffmpeg',
    'weight' => 19,
    'nice_parameter' => 'audiobitrate',
    'tool_parameter' => '-ab',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:63:{i:0;s:9:"a:31:{i:0";i:1;s:15:"s:9:"a:15:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:7:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:25:"s:18:"s:11:"s:5:"12200"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:25:"s:18:"s:11:"s:5:"40000"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:25:"s:18:"s:11:"s:5:"64000"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:15:"s:9:"s:3:"i:3""";i:28;s:4:"i:14";i:29;s:9:"s:3:"i:7"";i:30;s:4:"i:15";i:31;s:26:"s:19:"s:12:"s:6:"128000"""";i:32;s:4:"i:16";i:33;s:9:"s:3:"i:8"";i:34;s:4:"i:17";i:35;s:15:"s:9:"s:3:"i:4""";i:36;s:4:"i:18";i:37;s:9:"s:3:"i:9"";i:38;s:4:"i:19";i:39;s:26:"s:19:"s:12:"s:6:"192000"""";i:40;s:4:"i:20";i:41;s:10:"s:4:"i:10"";i:42;s:4:"i:21";i:43;s:15:"s:9:"s:3:"i:5""";i:44;s:4:"i:22";i:45;s:10:"s:4:"i:11"";i:46;s:4:"i:23";i:47;s:26:"s:19:"s:12:"s:6:"256000"""";i:48;s:4:"i:24";i:49;s:10:"s:4:"i:12"";i:50;s:4:"i:25";i:51;s:15:"s:9:"s:3:"i:6""";i:52;s:4:"i:26";i:53;s:10:"s:4:"i:13"";i:54;s:4:"i:27";i:55;s:26:"s:19:"s:12:"s:6:"512000"""";i:56;s:4:"i:28";i:57;s:10:"s:4:"i:14"";i:58;s:4:"i:29";i:59;s:13:"s:7:"s:1:"}""";i:60;s:4:"i:30";i:61;s:7:"s:1:"}"";i:62;s:1:"}";}',
    'default_value' => 128000,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Set the audio bitrate in bit/s@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_audiobitrate',
  );
  // Exported mediamosa tool ffmpeg params: audiosamplingrate.
  $mediamosa_tool_params['ffmpeg_audiosamplingrate'] = array(
    'tool' => 'ffmpeg',
    'weight' => 20,
    'nice_parameter' => 'audiosamplingrate',
    'tool_parameter' => '-ar',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => 'a:31:{i:0;s:9:"a:15:{i:0";i:1;s:14:"s:8:"a:7:{i:0"";i:2;s:3:"i:1";i:3;s:21:"s:14:"s:8:"a:3:{i:0""";i:4;s:3:"i:2";i:5;s:9:"s:3:"i:1"";i:6;s:3:"i:3";i:7;s:24:"s:17:"s:10:"s:4:"8000"""";i:8;s:3:"i:4";i:9;s:9:"s:3:"i:2"";i:10;s:3:"i:5";i:11;s:15:"s:9:"s:3:"i:1""";i:12;s:3:"i:6";i:13;s:9:"s:3:"i:3"";i:14;s:3:"i:7";i:15;s:25:"s:18:"s:11:"s:5:"22050"""";i:16;s:3:"i:8";i:17;s:9:"s:3:"i:4"";i:18;s:3:"i:9";i:19;s:15:"s:9:"s:3:"i:2""";i:20;s:4:"i:10";i:21;s:9:"s:3:"i:5"";i:22;s:4:"i:11";i:23;s:25:"s:18:"s:11:"s:5:"44100"""";i:24;s:4:"i:12";i:25;s:9:"s:3:"i:6"";i:26;s:4:"i:13";i:27;s:13:"s:7:"s:1:"}""";i:28;s:4:"i:14";i:29;s:7:"s:1:"}"";i:30;s:1:"}";}',
    'default_value' => 44100,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Set the audio sampling frequency@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_audiosamplingrate',
  );
  // Exported mediamosa tool ffmpeg params: ffmpeg.
  $mediamosa_tool_params['ffmpeg_ffmpeg'] = array(
    'tool' => 'ffmpeg',
    'weight' => 21,
    'nice_parameter' => 'ffmpeg',
    'tool_parameter' => '-ac',
    'min_value' => NULL,
    'max_value' => 2,
    'allowed_value' => NULL,
    'default_value' => 1,
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'The allowed value is between 0 and 2@default',
    'type_parameter' => 'SELECT',
    'machine_name' => 'ffmpeg_ffmpeg',
  );
  // Exported mediamosa tool image params: image.
  $mediamosa_tool_params['image_image'] = array(
    'tool' => 'image',
    'weight' => 0,
    'nice_parameter' => 'image',
    'tool_parameter' => '-resize',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => NULL,
    'default_value' => '',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Resize an image ({width}x{height}). Default the size of the original. See man convert for more information.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'image_image',
  );
  // Exported mediamosa tool image params: quality.
  $mediamosa_tool_params['image_quality'] = array(
    'tool' => 'image',
    'weight' => 0,
    'nice_parameter' => 'quality',
    'tool_parameter' => '-quality',
    'min_value' => NULL,
    'max_value' => NULL,
    'allowed_value' => NULL,
    'default_value' => '',
    'required' => 'FALSE',
    'created' => '2015-03-13 12:23:07',
    'description' => 'Quality is 0 (lowest image quality and highest compression) to 100 (best quality but least effective compression). The default quality is 75. See man convert for more information.',
    'type_parameter' => 'SELECT',
    'machine_name' => 'image_quality',
  );

  return $mediamosa_tool_params;
}
