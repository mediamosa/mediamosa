<?php
// $Id: CoderTestCase.php,v 1.2 2008/02/24 19:40:50 sun Exp $

require_once drupal_get_path('module', 'coder') .'/scripts/coder_format/coder_format.inc';

class CoderTestCase extends DrupalTestCase {
  function assertFormat($input, $expect) {
    $result = coder_format_string_all($input);
    $this->assertIdentical($result, $input);
  }
}

