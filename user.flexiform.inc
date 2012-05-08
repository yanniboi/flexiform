<?php
/**
 * @file entity getters and setters for the user entity
 */

/**
 * Simple entity getter for the current user. 
 *
 * Note that if the user is logged out the function returns false. If an entity
 * getter returns false, flexiform might create a new instance of it.
 *
 * @param $flexiform Flexiform : The Flexiform object
 * @param $entities array : An array of entities already loaded into the stack
 * @param $config : The config for this particular entity
 */ 
function flexiform_entity_getter_user_current_user($flexiform, $submission, $entities, $config) {
  global $user;
  
  if ($user->uid == 0) {
    return FALSE;
  }
  
  return user_load($user->uid);
}