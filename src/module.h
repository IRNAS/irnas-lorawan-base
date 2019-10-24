#ifndef module_h
#define module_h

// module includes
#include <Arduino.h>
#include "project_utils.h"

enum module_flags_e {
	M_IDLE,
	M_READ,
	M_SEND,
	M_RUNNING,
	M_ERROR
};

// virtual module class
class module {
public:
	virtual ~module() {};
	virtual String getName() {};
	virtual uint8_t get_global_id() {};
	virtual module_flags_e get_flags(void){};
	virtual uint8_t configure(uint8_t *data, size_t *size){};
	virtual uint8_t get_settings_length(){};
	virtual uint8_t set_downlink_data(uint8_t *data, size_t *size){};
	virtual module_flags_e scheduler(void){};
	virtual uint8_t initialize(void){};
	virtual uint8_t send(uint8_t *data, size_t *size){};
	virtual uint8_t read(void){};
	virtual void running(void){};
	virtual void event(main_share_t main_share){};
	virtual void print_data(void){};
};

//Tempalate module class
template <class myModuleType>
class myModule : public module {
public:

	myModuleType module;

	/**
	 * @brief Construct a new Module object wiht 3 general purpose parameters
	 * 
	 * @param id - global id of the module
	 * @param param_a 
	 * @param param_b 
	 * @param param_c 
	 */
	myModule(uint8_t id, uint8_t param_a = 0, uint8_t param_b = 0, uint8_t param_c = 0) {
		global_id = id;
		module.param_a=param_a;
		module.param_b=param_b;
		module.param_c=param_c;
	}

	/**
	 * @brief Get the Name object
	 * 
	 * @return String 
	 */
	String get_name(void) {
		return "name";//module.name;
	}

	/**
	 * @brief Get the global id object
	 * 
	 * @return uint8_t 
	 */
	uint8_t get_global_id(){
		return global_id;
	}

	/**
	 * @brief Get the flags object - universal method for module to triger global actions
	 * 
	 * @return module_flags_e flag value
	 */
	module_flags_e get_flags(void){
		return module.flags;
	}

	/**
	 * @brief Set the settings for the module
	 * 
	 * @param data 
	 * @param length 
	 * @return uint8_t response
	 */
	uint8_t configure(uint8_t *data, size_t *size){
		return module.configure(data, size);
	}

	/**
	 * @brief Get the settings length object
	 * 
	 * @return uint8_t 
	 */
	uint8_t get_settings_length(){
		return module.get_settings_length();
	}

	/**
	 * @brief Pass the downlink data to the module
	 * 
	 * @param data 
	 * @param length 
	 * @return uint8_t response
	 */
	uint8_t set_downlink_data(uint16_t *data, size_t *size){
		return module.set_downlink_data(data, size);
	}

	/**
	 * @brief scheduler object with the purpose of triggering internal functions of the module
	 * 
	 * @return module_flags_e response
	 */
	module_flags_e scheduler(void) {
		return module.scheduler();
	}

	/**
	 * @brief initialize the actions required in this module
	 * 
	 * @return uint8_t 
	 */
	uint8_t initialize(void){
		return module.initialize();
	}
	
	/**
	 * @brief send function that prepares the module information to be sent
	 * 
	 * @return uint8_t 
	 */
	uint8_t send(uint8_t *data, size_t *size){
		return module.send(data, size);
	}

	/**
	 * @brief performs reading of the values/sensors
	 * 
	 * @return uint8_t 
	 */
	uint8_t read(void){
		return module.read();
	}

	/**
	 * @brief performs running of the values/sensors
	 * 
	 * @return uint8_t 
	 */
	void running(void){
		module.running();
	}

	/**
	 * @brief print data of the module
	 * 
	 */
	void print_data(void){
		module.print_data();
	}


private:
	uint8_t global_id=3;
};

#endif
