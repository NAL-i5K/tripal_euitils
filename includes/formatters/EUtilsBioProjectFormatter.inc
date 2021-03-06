<?php

/**
 * Class EutilsBioProject Formatter.
 *
 * @ingroup formatters
 */
class EUtilsBioProjectFormatter extends EUtilsFormatter {

  private $elements = [];

  /**
   * Add the formatted data into a table.
   *
   * @param array $data
   *   The parsed XML data.
   *
   * @return array
   *   Drupal form elements array of each section in a fieldset.
   */
  public function format(array $data) {

    try {
      $this->formatBaseRecord($data);
      $this->formatAttributes($data['attributes']);
      $pubs = $data['linked_records']['pubs'];
      unset($data['linked_records']['pubs']);
      $this->formatLinkedRecords($data['linked_records']);
      $this->formatPublications($pubs);
    }
    catch (Exception $exception) {
      drupal_set_message($exception->getMessage());
    }

    return $this->elements;
  }

  /**
   * Handle the base Chado.analysis record.
   *
   * @param array $data
   *   From \EutilsAssemblyParser.
   */
  private function formatBaseRecord(array $data) {

    $header = ['Field', 'Value'];
    $accession = $data['accessions']['BioProject'] ?? NULL;

    $rows[] = ['Name', $data['name']];
    $rows[] = ['Description', $data['description']];

    if ($accession) {
      $rows[] = ['Accession', $accession];
    }

    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['base'] = [
      '#type' => 'fieldset',
      '#title' => 'Project',
      '#collapsible' => TRUE,
    ];
    $this->elements['base']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];
  }

  /**
   * Describe the Chado properties that will be associated with this analysis.
   *
   * @param array $attributes
   *   Array of attributes.  Should have stats and file keys.
   */
  private function formatAttributes(array $attributes) {

    if (empty($attributes)) {
      return;
    }
    $rows = [];
    $header = ['Name', 'Value'];

    foreach ($attributes as $key => $value) {
      $rows[] = [$key, $value];
    }

    $table = theme('table', ['rows' => $rows, 'header' => $header]);
    $this->elements['properties'] = [
      '#type' => 'fieldset',
      '#title' => 'Project Properties',
      '#collapsible' => TRUE,
    ];
    $this->elements['properties']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];

  }

  /**
   * Describes additional records that will be created.
   *
   * @param array $accessions
   *   the Accessions array from the EutilsBioProjectParser.
   */
  private function formatLinkedRecords(array $accessions) {

    if (empty($accessions)) {
      return;
    }

    $rows = [];
    $header = ['Type', 'Value'];

    foreach ($accessions as $type => $value) {
      // Ignored until project_biosample table is added.
      // However, we actually have it as biosample, so we need to fix that...
      if (strtolower($type) == 'biosample') {
        continue;
      }

      // Value is a string or an array of values.
      if (is_string($value)) {
        $link = $this->getDbLink($value, $type);
        $rows[] = [$type, $link];
      }

      if (is_array($value)) {
        foreach ($value as $singleValue) {
          $link = $this->getDbLink($singleValue, $type);
          $rows[] = [$type, $link];
        }
      }

    }

    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['links'] = [
      '#type' => 'fieldset',
      '#title' => 'Linked Records',
      '#collapsible' => TRUE,
    ];
    $this->elements['links']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];
  }

  /**
   * Formats publications.
   *
   * @param array $pubs
   *   Array of publication IDs.
   */
  private function formatPublications(array $pubs) {

    $this->elements['pubs'] = [
      '#type' => 'fieldset',
      '#title' => 'Publications',
      '#collapsible' => TRUE,
    ];

    $rows = [];
    $header = ['Pubmed ID'];

    foreach ($pubs as $accession) {

      // PMID is what tripal core sets up.
      $link = $this->getDbLink($accession, 'PMID');

      $row = [$link];

      $rows[] = $row;
    }

    if (empty($rows)) {
      $this->element['pubs']['notice'] = ['#markup' => 'No publications are associated with this Project.'];

    }
    $table = theme('table', ['rows' => $rows, 'header' => $header]);

    $this->elements['pubs']['table'] = [
      '#markup' => $table,
      '#type' => 'item',
    ];
  }

}
