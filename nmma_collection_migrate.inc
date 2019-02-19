<?php
class NMMACollectionMigration extends Migration {
	protected $json_file;
	protected $talker;
	protected $import_user;

  public function __construct($arguments) {
    parent::__construct($arguments);

    $this->description = t('Import Collection from FileMaker Cloud');
    $this->defaultLanguage = 'en';
		$this->import_user = variable_get('import_user');

		$fields = array(
			'recordId' => t('Record ID'),
			'Approved_for_Web' => t('Approved for Web'),
			'Main' => t('Main'),
			'Image_Ready' => t('Image ready'),
			'Artist_Birth' => t('Artist Birth'),
			'Artist_Death' => t('Artist Death'),
			'Artist_Country' => t('Artist Country'),
			'Accession_number' => t('Accession Number'),
			'Accession_Part' => t('Accession Part'),
			'Object_Type' => t('Object Type'),
			'Culture' => t('Culture'),
			'Artist_First_Name' => t('Artist First Name'),
			'Artist_Middle_Name' => t('Artist Middle Name'),
			'Artist_Last_Name' => t('Artist Last Name'),
			'Designer_Last_Name' => t('Designer Last Name'),
			'Designer_First_Name' => t('Designer First Name'),
			'Designer_Middle_Name' => t('Designer Middle Name'),
			'Creation_Month' => t('Creation Month'),
			'Photographer_First_Name' => t('Photographer First Name'),
			'Photographer_Last_Name' => t('Photographer Last Name'),
			'Photographer_Middle_Name' => t('Photographer Middle Name'),
			'Title' => t('Title'),
			'Phase_Style' => t('Phase Style'),
			'Translation' => t('Translation'),
			'Period' => t('Period'),
			'Region' => t('Region'),
			'Media' => t('Media'),
			'Edition_Run' => t('Edition Run'),
			'Creation_Year' => t('Creation Year'),
			'Dimensions' => t('Dimensions'),
			'Paper_Dimensions' => t('Paper Dimensions'),
			'Credit' => t('Credit'),
      'Image Name' => t('Image Name'),
      'permcoll_IMAGES~recordID::webImage' => t('Web Image Path'),
      'permcoll_IMAGES::webImageName' => t('Web Image Name'),
    );

		$images_destination = 'public://collection/images/';

		$this->talker = new FileMakerTalker();
		$this->json_file = $this->talker->get_items();
		// $this->json_file = $talker->json_folder . 'example-response.json';

		// because migration is the WORST call the query first write it to file
		// THEN use this because, uugggghhhhhh
		$this->source = new MigrateSourceList(
			new FileMakerListJSON('public://'.$this->json_file->filename),
			new FileMakerItemJSON('public://'.$this->json_file->filename, array()), $fields
		);
    $this->destination = new MigrateDestinationNode('nmma_collection_item_json');

		// Mapping
		$keySchema = MigrateDestinationNode::getKeySchema();

		$this->map = new MigrateSQLMap($this->machineName,
			array(
				'recordId' => array(
					'type' => 'varchar',
					'length' => 255,
				),
			),
			$keySchema
		);

		$this->addFieldMapping('uid')->defaultValue($this->import_user);
		$this->addFieldMapping('field_record_id', 'recordId');
		$this->addFieldMapping('title', 'Title')->callbacks(array($this, 'trimTitle'));
		$this->addFieldMapping('field_full_title', 'Title');
		$this->addFieldMapping('field_accession_number', 'Accession_number');
		$this->addFieldMapping('field_accession_part', 'Accession_Part');
		$this->addFieldMapping('field_approved_for_web', 'Approved_for_Web')->callbacks(array($this, 'computeApprovedForWeb'));
		$this->addFieldMapping('field_image_ready', 'Image_Ready')->callbacks(array($this, 'computeImageReady'));
		$this->addFieldMapping('field_artist_first_name', 'Artist_First_Name');
		$this->addFieldMapping('field_artist_middle_name', 'Artist_Middle_Name');
		$this->addFieldMapping('field_artist_last_name', 'Artist_Last_Name');
		$this->addFieldMapping('field_designer_first_name', 'Designer_First_Name');
		$this->addFieldMapping('field_designer_middle_name', 'Designer_Middle_Name');
		$this->addFieldMapping('field_designer_last_name', 'Designer_Last_Name');
		$this->addFieldMapping('field_photographer_first_name', 'Photographer_First_Name');
		$this->addFieldMapping('field_photographer_middle_name', 'Photographer_Middle_Name');
		$this->addFieldMapping('field_photographer_last_name', 'Photographer_Last_Name');
		$this->addFieldMapping('field_artist_birth', 'Artist_Birth');
		$this->addFieldMapping('field_artist_death', 'Artist_Death');
		$this->addFieldMapping('field_artist_country', 'Artist_Country');
		$this->addFieldMapping('field_category', 'Category');
		$this->addFieldMapping('field_category:create_term')->defaultValue(TRUE);
		$this->addFieldMapping('field_category:ignore_case')->defaultValue(TRUE);
		$this->addFieldMapping('field_culture', 'Culture');
		$this->addFieldMapping('field_culture:create_term')->defaultValue(TRUE);
		$this->addFieldMapping('field_culture:ignore_case')->defaultValue(TRUE);
		$this->addFieldMapping('field_object_type', 'Object_Type');
		$this->addFieldMapping('field_phase_style', 'Phase_Style');
		$this->addFieldMapping('field_period', 'Period');
		$this->addFieldMapping('field_period:create_term')->defaultValue(TRUE);
		$this->addFieldMapping('field_period:ignore_case')->defaultValue(TRUE);
		$this->addFieldMapping('field_creation_year', 'Creation_Year');
		$this->addFieldMapping('field_dimensions', 'Dimensions');
		$this->addFieldMapping('field_paper_dimensions', 'Paper_Dimensions');
		$this->addFieldMapping('field_edition_run', 'Edition_Run');
		$this->addFieldMapping('field_credit', 'Credit');
		$this->addFieldMapping('field_region', 'Region');
		$this->addFieldMapping('field_region:create_term');
		$this->addFieldMapping('field_region:ignore_case');
		$this->addFieldMapping('field_translation', 'Translation');
		$this->addFieldMapping('field_media', 'Media');

		// $this->addFieldMapping('field_item_image', 'image_name');
		$this->addFieldMapping('field_item_image:destination_dir')->defaultValue($images_destination);
		// use callback to find a matching file
		$this->addFieldMapping('field_item_image','image_name')->callbacks(array($this,'computeImageFileName'));
		// use existing files
		$this->addFieldMapping('field_item_image:file_replace')->defaultValue(FILE_EXISTS_REPLACE);
		$this->addFieldMapping('field_item_image:file_class')->defaultValue('MigrateFileUri');
		$this->addFieldMapping('field_item_image:image_alt','Title');
	}

	public function prepareRow($row) {
		if (parent::prepareRow($row) === FALSE) {
			return FALSE;
		}

		$accession_num_prop = 'Accession #';
		$accession_part_prop = 'Accession Part';
		$image_ready_prop = 'Image Ready';
		$artist_first_prop = 'Artist First Name';
		$artist_middle_prop = 'Artist Middle Name';
		$artist_last_prop = 'Artist Last Name';
		$designer_first_prop = 'Designer First Name';
		$designer_middle_prop = 'Designer Middle Name';
		$designer_last_prop = 'Designer Last Name';
		$photo_first_prop = 'Photographer First Name';
		$photo_middle_prop = 'Photographer Middle Name';
		$photo_last_prop = 'Photographer Last Name';
		$artist_birth_prop = 'Artist Birth';
		$artist_death_prop = 'Artist Death';
		$artist_country_prop = 'Artist Country';
		$object_type_prop = 'Object Type';
		$phase_style_prop = 'Phase Style';
		$creation_year_prop = 'Creation Year';
		$paper_dimensions_prop = 'Paper Dimensions';
		$edition_run_prop = 'Edition Run';
		$filename_prop = 'permcoll_IMAGES::fileName_W';
		$origin_prop = 'permcoll_IMAGE~web::IMAGE';

		$jar = new \GuzzleHttp\Cookie\CookieJar;
		$client = new \GuzzleHttp\Client();

		$image_name = $row->fieldData->$filename_prop;
		// Migration::displayMessage('image: ' . $image_uri);
		if(strlen($row->fieldData->$origin_prop)) {
			$resource = fopen('public://' . $image_name, 'w');
			$response = $client->request('GET', $row->fieldData->$origin_prop, ['cookies' => $jar, 'sink' => $resource]);
		}
		// $code = $response->getStatusCode(); // 200
		// $reason = $response->getReasonPhrase(); // OK

		$row->Title = $row->fieldData->Title;
		$row->Accession_number = $row->fieldData->$accession_num_prop;
		$row->Accession_Part = $row->fieldData->$accession_part_prop;
		$row->Approved_for_Web = $row->fieldData->Approved_for_Web;
		$row->Category = $row->fieldData->Category;
		$row->Image_Ready = $row->fieldData->$image_ready_prop;
		$row->Artist_First_Name = $row->fieldData->$artist_first_prop;
		$row->Artist_Middle_Name = $row->fieldData->$artist_middle_prop;
		$row->Artist_Last_Name = $row->fieldData->$artist_last_prop;
		$row->Designer_First_Name = $row->fieldData->$designer_first_prop;
		$row->Designer_Middle_Name = $row->fieldData->$designer_middle_prop;
		$row->Designer_Last_Name = $row->fieldData->$designer_last_prop;
		$row->Photographer_First_Name = $row->fieldData->$photo_first_prop;
		$row->Photographer_Middle_Name = $row->fieldData->$photo_middle_prop;
		$row->Photographer_Last_Name = $row->fieldData->$photo_last_prop;
		$row->Artist_Birth = $row->fieldData->$artist_birth_prop;
		$row->Artist_Death = $row->fieldData->$artist_death_prop;
		$row->Artist_Country = $row->fieldData->$artist_country_prop;
		$row->Culture = $row->fieldData->Culture;
		$row->Object_Type = $row->fieldData->$object_type_prop;
		$row->Phase_Style = $row->fieldData->$phase_style_prop;
		$row->Period = $row->fieldData->Period;
		$row->Creation_Year = $row->fieldData->$creation_year_prop;
		$row->Dimensions = $row->fieldData->Dimensions;
		$row->Paper_Dimensions = $row->fieldData->$paper_dimensions_prop;
		$row->Edition_Run = $row->fieldData->$edition_run_prop;
		$row->Credit = $row->fieldData->Credit;
		$row->Region = $row->fieldData->Region;
		$row->Translation = $row->fieldData->Translation;
		$row->Media = $row->fieldData->Media;
		$row->image_name = $row->fieldData->$filename_prop;
		$row->origin = $row->fieldData->$origin_prop;


		// Migration::displayMessage( print_r( $row, true ) );

		return true;
	}

	protected function computeApprovedForWeb($value) {
		$value = ($value === 'Web') ? 1 : 0;
		return $value;
	}

	protected function computeImageReady($value) {
		return intval($value);
	}

	protected function trimTitle($value) {
		return (strlen($value) > 255) ? substr($value, 0, 255) : $value;
	}

	public function postImport() {
		parent::postImport();
		// file_delete($this->json_file, TRUE);
	}

	protected function computeImageFileName($value) {
		$out_value = '';

		if(!empty($value)) {
			$wrapper = file_stream_wrapper_get_instance_by_uri('public://'.$value);
			$image_incoming = $wrapper->realpath();
			$out_value = mb_check_encoding($image_incoming, 'UTF-8') ? $image_incoming : utf8_encoding($image_incoming);
		}
		return $out_value;
	}
}