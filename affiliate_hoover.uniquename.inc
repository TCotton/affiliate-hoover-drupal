<?php

function affiliate_hoover_in_array_like($referencia, $array){
    
    foreach($array as $ref) {
        if (stristr($referencia, $ref)) {
            return true;
        }
    }
    
    return false;
    
}


// checks that URL item is actual form. returns false if not and is used in a 404 message below
function affiliate_hoover_unique_name($args){
    
    $aff = variable_get('affiliate_hoover');
    
    $result = affiliate_hoover_in_array_like($args, $aff);
    
    if (!$result) {
        
        return drupal_not_found();
        drupal_exit();
        
    } else {
        
        return drupal_get_form('affiliate_hoover_uniquename_build_form');
        
    }
    // end if
    
}
// end if affiliate_hoover_unique_name()

function affiliate_hoover_uniquename_build_form($form, &$form_state){
    
    $form['affiliate_hoover_unique_name_form'] = array(
    '#type' => 'item', 
    '#title' => t('Feed details for ' . arg(4)),
    '#description' => 'Upload a CSV file here',
    '#attributes' => array('enctype' =>
    'multipart/form-data'),
    //'#required' => true,
  );
  
  $form['affiliate_hoover_unique_name_form']['first_part'] = array(
    '#type' => 'fieldset', 
    '#title' => '',
    '#description' => '',
  );
    
  $form['affiliate_hoover_unique_name_form']['first_part']['ah_file'] = array(
    '#type' => 'file', 
    '#title' => t('Upload file'), 
    '#description' => t('<p>Upload your CSV file here.</p><p>The maximum file upload is ' .
    (file_upload_max_size() / 1048576) . 'MBs</p>'),
    
  );

  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => t('Submit'),
  );

  return $form;
    
    /*
    $form['affiliate_hoover_uniquename'] =
    array('#title' => t('Feed details for ' . arg(4)),
    '#type' => 'fieldset', '
#required' => true,
    '#attributes' => array('enctype' =>
    'multipart/form-data'),
    '#description' => t('<p>Upload your CSV file here.</p><p>The maximum file upload is ' .
    (file_upload_max_size() / 1048576) . 'MBs</p>'), );
    $form['affiliate_hoover_uniquename']['upload'] = array('#type' => 'fieldset',
    '#title' => t('Name'), );
    $form['affiliate_hoover_uniquename']['upload']['ah_file'] = array('#type' =>
    'file', '#title' => t('First name'),
    );
    
    $form['submit'] = array('#type' => 'submit', '#value' => 'Upload', );
    
    return $form;
    #*/
    
}



/**
* Validation for unmanaged file form.
*/
function affiliate_hoover_uniquename_build_form_validate($form, &$form_state){
    
    $validators = array('file_validate_extensions' => array('csv'),
    //'assets_validate_not_in_filename' => array('test.'),
    
    'file_validate_size' => array(file_upload_max_size()),
    //'affiliate_hoover_uniquename_build_form_file_validate' => array('test'),
    
    );
    
    // Additional validation functions in core:
    // file_validate_size();
    // file_validate_is_image();
    // file_validate_extensions();
    // file_validate_image_resolution();
    // file_validate_name_length();
    
    
    $file = file_save_upload('ah_file', $validators, FALSE, FILE_EXISTS_REPLACE);
    
    //$url = file_build_uri('public://affiliate_hoover/' . $record->fileName );
    
    //var_dump($url);
    
    if (isset($file)) {
        // File upload was attempted.
        if ($file) {
            // Put the temporary file in form_values so we can save it on submit.
            $form_state['values']['ah_file'] = $file;
        } else {
            form_set_error('ah_file', t('The file could not be uploaded.'));
        }
    }
}



/**
* Validation for unmanaged file form.
*/

/*

function affiliate_hoover_uniquename_build_form_validate($form, &$form_state)
{
    
    // We can't set #required for a file input, so this is how we would check for a value
    if (!isset($form_state['values']['ah_file']) || !is_numeric($form_state['values']['ah_file'])) {
        
        form_set_error('ah_file', t('Please select a file to upload.'));
        
    }
    
}

*/


/**
* Submit function for unmanaged file form.
*/
function affiliate_hoover_uniquename_build_form_submit($form, &$form_state){
    
    // Check if a file was uploaded.
    if ($file = $form_state['values']['ah_file']) {
        
        // Create the directory if it doesn't exist
        $directory = 'public://affiliate_hoover';
        file_prepare_directory($directory, FILE_CREATE_DIRECTORY);
        
        // Copy the file over.
        $filename = file_unmanaged_copy($file->uri, $directory, FILE_EXISTS_REPLACE);
        
        //var_dump($filename);
        
        $header_array = affiliate_hoover_parse_csv_head($file->uri);
        
        $header_array_amend = $header_array;
        //count total number of entries
        $num_rows = affiliate_hoover_count_csv_rows($file->uri);
        foreach($header_array_amend as $key => $value) {
            $header_array_amend['[#'.$key.'#]'] = $value;
            unset($header_array_amend[$key]);
        }
        // end foreach
        
        // check databse table to see if the entry already exists.
        // If not create new entry
        // If it does then update
        
        $result = db_query('SELECT n.name, n.fileName FROM {ah_feed_details} n WHERE n.name = :name', array(':name' => arg(4)))->fetchObject();
        
        if ($result != FALSE) {
            
            //$record = $result->fetchObject();
            
            if ($result->fileName != $file->filename) {
                
                // delete old file if the file name for the feed has changed
                
                drupal_unlink('public://affiliate_hoover/' . $record->fileName );
                
            }
            
            db_update('ah_feed_details')
            ->fields(array('fileName' => $file->filename,
            'header_array' => serialize($header_array),
            'header_array_amend' => serialize($header_array_amend),
            'num_rows' => $num_rows,
            'url' => $filename,
            ))
            ->condition('name', arg(4))
            ->execute();
            
        } else {
            
            // create new entry
            
            db_insert('ah_feed_details')
            ->fields(array('name' => arg(4),
            'fileName' => $file->filename,
            'header_array' => serialize($header_array),
            'header_array_amend' => serialize($header_array_amend),
            'num_rows' => $num_rows,
            'url' => $filename,
            ))->execute();
            
        }
        
        drupal_set_message(t("Your file has been uploaded!"));
        
    }
    
}


function affiliate_hoover_parse_csv_head($file){
    
    $csv = new File_CSV_DataSource;
    
    if ($csv->load($file)) {
        
        return $csv->getHeaders();
        
    } else {
        
        return FALSE;
        
    }
    // end if ($csv->sadfsad
}


function affiliate_hoover_count_csv_rows($file){
    
    $csv = new File_CSV_DataSource;
    
    if ($csv->load($file)) {
        
        return $csv->countRows();
        
    } else {
        
        return FALSE;
        
    }
    // end if ($csv->sadfsad
}


/**
* Implements hook_file_validate().
*
* We're testing to see if the string 'test.' exists in the file name.
*/
/*
function affiliate_hoover_uniquename_build_form_file_validate($file, $string = 'test.') {
$errors = array();
if (strpos($file->filename, $string) !== FALSE) {
$errors[] = t('The string "%string" is reserved, please remove this string from your filename and try again.',
array('%string' => $string));
}
return $errors;
}
*/


function affiliate_hoover_uniquename_title_callback($args){
    
    //var_dump($args);
    return t($args);
    
    
}
