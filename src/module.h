#ifndef module_h
#define module_h

// module includes
#include <Arduino.h>

// virtual module class
class module {
public:
	virtual ~module() {};
	virtual String getName() {};
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
	myModule(uint8_t id, uint8_t param_a, uint8_t param_b, uint8_t param_c) {
		global_id = id;
		module.param_a=param_a;
		module.param_b=param_b;
		module.param_c=param_c;
	};

	/**
	 * @brief Get the Name object
	 * 
	 * @return String 
	 */
	String get_name(void) {
		return module.name;
	};

	/**
	 * @brief Get the flags object - universal method for module to triger global actions
	 * 
	 * @return uint8_t flag value
	 */
	uint8_t get_flags(void){
		return module.get_flags;
	}

	/**
	 * @brief Set the settings for the module
	 * 
	 * @param data 
	 * @param length 
	 * @return uint8_t response
	 */
	uint8_t set_settings(uint16_t *data, uint16_t length){
		return module.set_settings(data, length);
	}

	/**
	 * @brief Pass the downlink data to the module
	 * 
	 * @param data 
	 * @param length 
	 * @return uint8_t response
	 */
	uint8_t set_downlink_data(uint16_t *data, uint16_t length){
		return module.set_downlink_data(data, length);
	}

	/**
	 * @brief scheduler object with the purpose of triggering internal functions of the module
	 * 
	 * @return uint8_t response
	 */
	uint8_t scheduler() {
		return module.scheduler();
	};

	/**
	 * @brief initialize the actions required in this module
	 * 
	 * @return uint8_t 
	 */
	uint8_t initialize(){
		return module.initialize();
	}
	
	/**
	 * @brief send function that prepares the module information to be sent
	 * 
	 * @return uint8_t 
	 */
	uint8_t send(){
		return module.send();
	}
private:
	// globally unique id, corresponds typically to LoRaWAN port
	uint8_t global_id;

};

#endif
