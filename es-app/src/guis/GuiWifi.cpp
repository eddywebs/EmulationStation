// Easy Network Manager written for Linux/RPi version of ES by Jacob Karleskint
// ---
// Heavely uses system commands to get wifi and network info with elevated permissions
// wificonnect must be compiled and put in ~/.emulationstation/app for this to
// fully work.  [ source found in {source_dir}/external/wifi ]
// Want to add options to remove networks and manual wifi setup.


#include "EmulationStation.h"
#include "guis/GuiWifi.h"
#include "guis/GuiWifiConnect.h"
#include "Window.h"
#include "Sound.h"
#include "Log.h"
#include "Settings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSettings.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>

#include "components/ButtonComponent.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "components/MenuComponent.h"
#include "guis/GuiTextEditPopup.h"

bool getWifiState(){
		// Check if system is already connected to a network and bail
			FILE *fp;
			char val[7];
			fp = popen("cat /sys/class/net/wlan0/flags", "r");
			fgets(val, sizeof(val), fp);
			LOG(LogWarning) << val;

			if( strcmp(val, "0x1002") == 0 ){
				LOG(LogWarning) << "wifi is off";
				return false;
			}

			return true;

	}

void switchWan(bool value ){ 
	LOG(LogWarning) << value;  

	if(value == 1){
		system("sudo ifconfig wlan0 up");
		LOG(LogWarning) << "turning wifi up";

	}
	else{
	
		system("sudo ifconfig wlan0 down ");
		LOG(LogWarning) << "turning wifi down";
		
	}

}

GuiWifi::GuiWifi(Window* window) : GuiComponent(window), mMenu(window, "NETWORK SETTINGS")
{
	// MAIN WIFI MENU

	// CONNECT TO NEW NETWORK >
	// SHOW CURRENT WIFI INFO >
	// ETHERNET INFO >
	// x MANUAL SETUP > [todo?]
	// SAVED NETWORKS > [todo]
	// TURN WIFI ON/OFF

	// [version]



	std::string wificonnect_path = getHomePath() + "/.emulationstation/app/wifi/./wificonnect";

	addEntry("CONNECT TO NEW WIFI", mMenu.getTextColor(), true, 
		[this, window] { 

			auto s = new GuiSettings(mWindow, "AVAILABLE NETWORKS");
			// Holds the password the user types in.
			std::shared_ptr<GuiComponent> password;

			// Check if system is already connected to a network and bail
			FILE *fp;
			char path[1035];
			fp = popen("ifconfig wlan0 | grep \"inet addr:\"", "r");
			fgets(path, sizeof(path), fp);
			LOG(LogWarning) << "This is a log test \n";
			LOG(LogWarning) << path;
			// whis there the need for this check ? 
			// if (std::strlen(path) > 1) {
			// 	mWindow->pushGui(new GuiMsgBox(mWindow,
			// 		"Cannot scan for networks when the device is already connected to one.  You can delete the current network in SAVED NETWORKS",
			// 		"OK"));
			// 	LOG(LogWarning) << "Attemped to scan for new wireless networks but wlan0 already has an IP address.";
			// 	return;
			// }

			// no need to bring down wireless to refresh
			// system("sudo ifconfig wlan0 down");
			// system("sudo ifconfig wlan0 up");

			// sleep(5); //wait for 5 secs
			// //system("sudo ifdown wlan0 &>/dev/null");

			// dump iwlist into a memory file
			fp = popen("sudo iwlist wlan0 scanning | grep 'SSID\\|Channel:\\|Encryption\\|Quality'", "r");

			// Variables
			std::string wSSID[24];
			std::string wChannel[24];
			std::string wQuality[24];
			bool bEncryption[24];

			int ssidIndex = 0;
			int channelIndex = 0;
			int qualityIndex = 0;
			int encryptionIndex = 0;

			// Read that file to get info and parse into variables
			std::string currentLine;
			std::size_t found;

			while (fgets(path, sizeof(path), fp) != NULL) {
				currentLine = path;

				// See if this network is encrypted
				if (currentLine.find("Encryption key:on") != std::string::npos){
					bEncryption[encryptionIndex] = true;
					encryptionIndex++;
				}
				else {
					if (currentLine.find("Encryption key:off") != std::string::npos) {
						bEncryption[encryptionIndex] = false;
						encryptionIndex++;
					}
				}

				found = currentLine.find("ESSID");
				if (found != std::string::npos) {
					std::string ssid = currentLine;
					ssid = ssid.substr(found + 6);
					int trim = ssid.find("\n");
					ssid = ssid.substr(1, trim - 2);
					if (ssid == "" || ssid == " ") ssid = "[Hidden Network]";
					wSSID[ssidIndex] = ssid;
					ssidIndex++;
					continue;
				}

				found = currentLine.find("Channel");
				if (found != std::string::npos) {

					std::string channel = currentLine;
					channel = channel.substr(found + 8) + " ";
					int trim = channel.find("\n");
					channel = channel.substr(0, trim - 1);
					wChannel[channelIndex] = channel;
					channelIndex++;
					continue;
				}

				found = currentLine.find("Quality");
				if (found != std::string::npos) {
					std::string quality = currentLine;
					quality = quality.substr(found + 29, 2);
					int trim = quality.find("\n");
					quality = quality.substr(0, trim - 1);
					wQuality[qualityIndex] = quality;
					qualityIndex++;
					continue;
				}
			}

			ComponentListRow row;

			// For loop all networks out
			int color = mMenu.getTextColor();
			//for (int i = ssidIndex - 1; i > -1; i--) {
			for (int i = 0; i < ssidIndex; i++){

				// Signal strength color setter
				std::string qual = wQuality[i];
				std::string sigText = "|";
				int intQuality = std::atoi(qual.c_str());
				if (intQuality >= 85) color = 0xCE0000FF;
				else if (intQuality >= 75) {
					color = 0xF24207FF;
					sigText = "| |";
				}
				else if (intQuality >= 65) {
					color = 0x33F207FF;
					sigText = "| | |";
				}
				else if (intQuality > 10) {
					color = 0x078DF2FF;
					sigText = "| | | |";
				}

				// Encrpytion color
				int encryptionColor = 0x77BB77FF;
				if (bEncryption[i]) encryptionColor = 0xFF7777FF;

				// Add the Wifi information per row
				row.addElement(std::make_shared<TextComponent>(mWindow, "" + wSSID[i], Font::get(FONT_SIZE_MEDIUM), encryptionColor), true);
				//row.addElement(std::make_shared<TextComponent>(mWindow, "C: " + wChannel[i], Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true); // shows channel.  (buggy)

				// Create signal graph, and align it to the right.
				auto signal_comp = std::make_shared<TextComponent>(mWindow, "" + sigText, Font::get(FONT_SIZE_MEDIUM), color);
				signal_comp->setAlignment(ALIGN_RIGHT);
				row.addElement(signal_comp, true);

				//Create what to do whne this network is clicked.
				std::string pSSID = wSSID[i];
				bool encrypt = bEncryption[i];
				row.makeAcceptInputHandler( [this, pSSID, encrypt] {
					mWindow->pushGui(new GuiWifiConnect(mWindow, pSSID, encrypt));
				});

				s->addRow(row);
				row.elements.clear();
				
			}

			// If no networks could be found, let the user know
			if (ssidIndex < 1) {
				row.elements.clear();
				auto no_network = std::make_shared<TextComponent>(mWindow, "NO NETWORKS COULD BE FOUND.", Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
				no_network->setAlignment(ALIGN_CENTER);
				row.addElement(no_network, true);
				s->addRow(row);
			}


			mWindow->pushGui(s);
	});

	addEntry("SHOW CURRENT WIFI INFO", mMenu.getTextColor(), true,
		[this] {
			// dump iwlist and ifconfig into a memory file through pipe
			FILE *iwList;
			FILE *wIPP;
			char iw[1035];
			char wip[1035];

			// Specific Linux commands. 
			iwList = popen("iwlist wlan0 scanning | grep 'SSID\\|Frequency\\|Channel\\|IEEE\\|Quality'", "r");
			wIPP = popen("ifconfig wlan0 | grep 'inet addr'", "r");

			// Variables
			std::string wSSID;
			std::string wChannel;
			std::string wQuality;
			std::string wIP;

			// Read the pipe to get info and parse into variables
			std::string currentLine;
			std::size_t found;

			while (fgets(iw, sizeof(iw), iwList) != NULL) {
				currentLine = iw;

				found = currentLine.find("ESSID");
				if (found != std::string::npos) {
					wSSID = currentLine;
					wSSID = wSSID.substr(found + 6);		// Trim out front garbage
					int trim = wSSID.find("\n");
					wSSID = wSSID.substr(1, trim -2);		// Trim out \n and "s
				}

				found = currentLine.find("Channel");
				if (found != std::string::npos) {
					wChannel = currentLine;
					wChannel = wChannel.substr(found + 8) + " ";
					int trim = wChannel.find("\n");
					wChannel = wChannel.substr(0, trim - 1);	// trim out \n and ending )
				}

				found = currentLine.find("Quality");
				if (found != std::string::npos) {
					wQuality = currentLine;
					wQuality = wQuality.substr(found);
					int trim = wQuality.find("\n");
					wQuality = wQuality.substr(0, trim);		// trim out \n
				}
			}

			// Open up Grepped wlan0 ip file
			while (fgets(wip, sizeof(wip), wIPP) != NULL) {
				currentLine = wip;

				// find location of ipv4 address
				found = currentLine.find("inet addr");
				if (found != std::string::npos) {
					// Format string to rip out uneeded data
					wIP = currentLine;
					wIP = wIP.substr(found + 10, 42);
					int trimBcast = wIP.find("Bcast");
					wIP = wIP.substr(0, trimBcast - 1);
				}
			}

			auto s = new GuiSettings(mWindow, "CURRENT WIFI NETWORK");

			// ESSID
			auto show_ssid = std::make_shared<TextComponent>(mWindow, "" + wSSID, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
			s->addWithLabel("Network Name", show_ssid);

			auto show_ip = std::make_shared<TextComponent>(mWindow, "" + wIP, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
			s->addWithLabel("IPv4", show_ip);

			auto show_channel = std::make_shared<TextComponent>(mWindow, "" + wChannel, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
			s->addWithLabel("Channel", show_channel);

			auto show_quality = std::make_shared<TextComponent>(mWindow, "" + wQuality, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
			s->addWithLabel("Signal", show_quality);

			mWindow->pushGui(s);
	});

	addEntry("SHOW ETHERNET DETAILS", mMenu.getTextColor(), true,
		[this] {
		// dump iwlist and ifconfig into a memory file through pipe
		FILE *wIPP;
		char wip[1035];

		wIPP = popen("ifconfig eth0", "r");

		// Variables
		std::string wIP;
		std::string wMac;
		std::string wRX;
		std::string wTX;

		// Read the pipe to get info and parse into variables
		std::string currentLine;
		std::size_t found;

		// Open up Grepped wlan0 ip file
		while (fgets(wip, sizeof(wip), wIPP) != NULL) {
			currentLine = wip;

			// find location of ipv4 address
			found = currentLine.find("inet addr");
			if (found != std::string::npos) {
				// Format string to rip out uneeded data
				wIP = currentLine;
				wIP = wIP.substr(found + 10, 42);
				int trimBcast = wIP.find("Bcast");
				wIP = wIP.substr(0, trimBcast - 2);
			}

			found = currentLine.find("HWaddr");
			if (found != std::string::npos) {
				wMac = currentLine;
				wMac = wMac.substr(found + 6, 18);
			}

			found = currentLine.find("RX bytes");
			if (found != std::string::npos) {
				int trim = currentLine.find("\n");
				wRX = currentLine.substr(0, trim - 2);
				trim = wRX.find("TX");
				wTX = wRX.substr(trim);
				wRX = wRX.substr(0, trim - 1);
			}
		}

		auto s = new GuiSettings(mWindow, "CURRENT NETWORK INFO");

		// Build window rows
		auto show_ip = std::make_shared<TextComponent>(mWindow, "" + wIP, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
		s->addWithLabel("IPv4", show_ip);

		auto show_mac = std::make_shared<TextComponent>(mWindow, "" + wMac, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
		s->addWithLabel("MAC", show_mac);

		auto show_data = std::make_shared<TextComponent>(mWindow, "" + wRX + "\n     " + wTX + ")", Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor());
		show_data->setAlignment(ALIGN_CENTER);
		s->addWithLabel("DATA", show_data);

		mWindow->pushGui(s);
	});

	addEntry("SAVED NETWORKS", mMenu.getTextColor(), true, [this, wificonnect_path] {
		// Grab network list from wpa_supplicant
		FILE *wIPP;
		char wip[1035];
		std::string currentLine;
		bool destroySelf = false;

		LOG(LogWarning) << "wificonnect_path is: "+wificonnect_path;
		std::string command = "sudo " + wificonnect_path + " --list";

		// Check if wificonnect exists
		if (!boost::filesystem::exists(wificonnect_path)) {
			mWindow->pushGui(new GuiMsgBox(mWindow, "wificonnect is missing.  This is used to send wifi info to wpa_supplicant.  This should be in ~/.emulationstation/app/wifi"));
			return;
		}

		wIPP = popen(command.c_str(), "r");

		auto s = new GuiSettings(mWindow, "SAVED NETWORKS");

		ComponentListRow row;
		int netid = 1;

		while (fgets(wip, sizeof(wip), wIPP) != NULL) {
			currentLine = wip;
			currentLine = currentLine.substr(0, currentLine.length() - 1);

			row.addElement(std::make_shared<TextComponent>(mWindow, "Network: ", Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor()), true);
			row.addElement(std::make_shared<TextComponent>(mWindow, "" + currentLine, Font::get(FONT_SIZE_MEDIUM), mMenu.getTextColor()), true);

			row.makeAcceptInputHandler([this, currentLine, wificonnect_path, s] {
				mWindow->pushGui(new GuiMsgBox(mWindow, "DO YOU WISH TO DELETE THIS NETWORK FROM THIS DEVICE?", "YES", [this, currentLine, wificonnect_path, s] {
					// Delete network
					FILE *delNetwork;
					char pi[1035];
					std::string cLine;
					std::string command = "sudo " + wificonnect_path + " --remove \"" + currentLine + "\"";
					delNetwork = popen(command.c_str(), "r");

					while (fgets(pi, sizeof(pi), delNetwork) != NULL) {
						if (pi[0] == '0') {
							mWindow->pushGui(new GuiMsgBox(mWindow, "Succesfully deleted network", "OK"));
							delete s;
						}
						else {
							mWindow->pushGui(new GuiMsgBox(mWindow, "Could not delete network.", "OK"));
							LOG(LogError) << "[WifiGui] " << pi;
						}
					}
				}, "NO"));
			});

			s->addRow(row);
			row.elements.clear();
			netid++;
		}

		mWindow->pushGui(s);
	});



	addEntry("TURN WIFI ON/OFF", mMenu.getTextColor(), true, [this] {
		// mWindow->pushGui(new GuiMsgBox(mWindow, "Turn Wifi On or Off?", "ON", 
		// 	[] { system("sudo ifconfig wlan0 up"); },
		// 	"OFF", 
		// 	[] { system("sudo ifconfig wlan0 down"); },
		// 	"Cancel",
		// 	nullptr
		// ));
//Settings::getInstance()->setBool("SaveGamelistsOnExit", save_gamelists->getState());
		auto s = new GuiSettings(mWindow, "Wifi status");

			// gamelists
			auto save_gamelists = std::make_shared<SwitchComponent>(mWindow);
			save_gamelists->setState(getWifiState());
			s->addWithLabel("Wifi", save_gamelists);
			s->addSaveFunc([save_gamelists] { switchWan(save_gamelists->getState()); });
		mWindow->pushGui(s);	

	});

	


	mMenu.setFooter("GUIWIFI V 0.51");

	addChild(&mMenu);

	setSize(mMenu.getSize());
	setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

void GuiWifi::onSizeChanged()
{
}

void GuiWifi::addEntry(const char* name, unsigned int color, bool add_arrow, const std::function<void()>& func)
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	
	// populate the list
	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, name, font, color), true);

	if(add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);
		row.addElement(bracket, false);
	}
	
	row.makeAcceptInputHandler(func);

	mMenu.addRow(row);
}



bool GuiWifi::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo("b", input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiWifi::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", "choose"));
	prompts.push_back(HelpPrompt("a", "select"));
	prompts.push_back(HelpPrompt("start", "close"));
	return prompts;
}
