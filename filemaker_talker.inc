<?php
  class FileMakerTalker {
    protected $api_server;
    protected $api_database;
    protected $api_layout;
    protected $api_auth;

    public function __construct() {
      $this->api_server = variable_get('api_server');
      $this->api_database = variable_get('api_db');
      $this->api_layout = variable_get('api_layout');
      $this->api_auth = variable_get('api_auth');
    }

    public function getToken() {

      $request = new HttpRequest();
      $request->setUrl("https://$this->api_server/fmi/data/v1/databases/$this->api_database/sessions");
      $request->setMethod(HTTP_METH_POST);

      $request->setHeaders(array(
        'Postman-Token' => '9f053441-448b-4b2d-9c2b-ecb56d7eb7c6',
        'Cache-Control' => 'no-cache',
        'Authorization' => $this->api_auth,
        'Content-Type' => 'application/json'
      ));

      $request->setBody('{}');

      try {
        $response = $request->send();
        $body = $response->getBody();
        watchdog('migrate_logging', '<pre>' . $body . '</pre>', WATCHDOG_NOTICE);
      } catch (HttpException $ex) {
        watchdog('migrate_logging', '<pre>error: ' . $ex . '</pre>', WATCHDOG_NOTICE);
        echo $ex;
      }

      return $body;
    }
  }

  class FileMakerListJSON extends MigrateListJSON {
    /**
     * The default implementation assumes the IDs are top-level array elements,
     * but the array elements are the data items - we need to look inside them
     * for the IDs.
     */
    protected function getIDsFromJSON(array $data) {
      $ids = array();
      foreach ($data['response']['data'] as $item) {
        $ids[] = $item['recordId'];
      }
      return $ids;
    }

    /**
     * If this is the case then the count will be off as well and we will have to overriden as well.
     */
    public function computeCount() {
      $count = 0;
      if (empty($this->httpOptions)) {
        $json = file_get_contents($this->listUrl);
      }
      else {
        $response = drupal_http_request($this->listUrl, $this->httpOptions);
        $json = $response->data;
      }
      if ($json) {
        $data = drupal_json_decode($json);
        watchdog('migrate_logging', $data, WATCHDOG_NOTICE);

        if ($data) {
          $count = count($data['response']['data']);
        }
      }
      return $count;
    }
  }

  class FileMakerJSONReader extends MigrateJSONReader {
    /**
     * Implements MigrateJSONReader::next().
     */
    public function next() {
      migrate_instrument_start('MigrateJSONReader::next');

      $this->currentElement = $this->currentId = NULL;

      // Open the file and position it if necessary
      if (!$this->fileHandle) {
        $this->fileHandle = fopen($this->url, 'r');
        if (!$this->fileHandle) {
          Migration::displayMessage(t('Could not open JSON file !url',
            array('!url' => $this->url)));
          return;
        }

        // We're expecting an array of characters, so the first character should be [.
        $char = $this->getNonBlank();
        // Ning exports are wrapped in bogus (), so skip a leading (
        if ($char == '(') {
          $char = $this->getNonBlank();
        }
        if ($char != '[') {
          Migration::displayMessage(t('!url is not a JSON file containing an array of objects',
            array('!url' => $this->url)));
          return;
        }
      }

      // We expect to be positioned either at an object (beginning with {) or
      // the end of the file (we should see a ] indicating this). Or, an
      // object-separating comma, to be skipped. Note that this treats
      // commas as optional between objects, which helps with processing
      // malformed JSON with missing commas (as in Ning exports).
      $c = $this->getNonBlank();
      if ($c == ',') {
        $c = $this->getNonBlank();
      }
      // Ning sometimes emits a ] where there should be a comma.
      elseif ($c == ']') {
        $c = $this->getNonBlank();
        if ($c != '{') {
          $c = NULL;
        }
      }
      // We expect to be at the first character of an object now.
      if ($c == '{') {
        // Start building a JSON string for this object.
        $json = $c;
        // Look for the closing }, ignoring brackets in strings, tracking nested
        // brackets. Watch out for escaped quotes, but also note that \\" is not
        // an escaped quote.
        $depth = 1;
        $in_string = FALSE;
        $in_escape = FALSE;
        while (($c = fgetc($this->fileHandle)) !== FALSE) {
          $json .= $c;
          if ($in_string) {
            // Quietly accept an escaped character
            if ($in_escape) {
              $in_escape = FALSE;
            }
            else {
              switch ($c) {
                // Unescaped " means end of string
                case '"':
                  $in_string = FALSE;
                  break;
                // Unescaped \\ means start of escape
                case '\\':
                  $in_escape = TRUE;
                  break;
              }
            }
          }
          else {
            // Outside of strings, recognize {} as depth changes, " as start of
            // string.
            switch ($c) {
              case '{':
                $depth++;
                break;
              case '}':
                $depth--;
                break;
              case '"':
                $in_string = TRUE;
                break;
            }
            // We've found our match, exit the loop.
            if ($depth < 1) {
              break;
            }
          }
        }

        // Turn the JSON string into an object.
        $this->currentElement = json_decode($json);
        $this->currentId = $this->currentElement->{$this->idField};
      }
      else {
        $this->currentElement = NULL;
        $this->currentId = NULL;
      }
      migrate_instrument_stop('MigrateJSONReader::next');
    }
  }

  class FileMakerItemJSON extends MigrateItemJSON {
    protected $data = array();
    /**
     * Two problems with the default getItem() - it reads and parses the JSON on
     * each call, which we don't want to do for multiple items in the file; and,
     * we need to retrieve a given item from the file according to its 'id' element.
     */
    public function getItem($id) {
      // We cache the parsed JSON at $this->data.
      if (empty($this->data)) {
        $data = $this->loadJSONUrl($this->itemUrl);
        if ($data) {
          // Let's index the array by the ID for easy retrieval.

          foreach ($data as $key => $item) {
            if($key == 1) {
              Migration::displayMessage( print_r( $item, true ) );
            }
            $this->data[$item['recordId']] = $item;
          }
        }
        else {
          // Error-handling here....
        }
      }
      // Return the requested item
      if (isset($this->data[$id])) {
        return $this->data[$id];
      }
      else {
        return NULL;
      }
    }
  }