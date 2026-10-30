#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cctype>
#include <stdexcept>

using namespace std;

class SystemException : public std::runtime_error {
public:
    explicit SystemException(const string& message) 
        : std::runtime_error(message) {}
};

class Seat { // Base class
private:
    string seatNumber;
    double price;
    bool isReserved;
    static int SeatsCount;

public:
    Seat() : Seat("00", 0.0, false) {} 

    Seat(string number, double seatPrice, bool reserved = false):
        seatNumber(number), price(seatPrice), isReserved(reserved) {
        SeatsCount++;
    }

    virtual ~Seat() {
        if (SeatsCount > 0) {
            SeatsCount--;
        }
    }

    string getSeatNumber() const { return seatNumber; }
    double getPrice() const { return price; }
    bool getIsReserved() const { return isReserved; }

    void setReserved(bool status) { isReserved = status; }

    void setPrice(double newPrice) {
        if (newPrice < 0) {
            throw SystemException("Price cannot be negative.");
        }
        price = newPrice;
    }

    static int getSeatsCount() { return SeatsCount; }

    virtual string getSeatClass() const = 0;

    virtual void displaySeat() const {
        cout << left << setw(12) << seatNumber
             << setw(15) << getSeatClass()
             << "$" << setw(14) << fixed << setprecision(2) << price
             << (isReserved ? "RESERVED" : "AVAILABLE") << endl;
    }
};

int Seat::SeatsCount = 0;

class Business : public Seat {
private:
    bool isWindow;

public:
    Business(string number, double seatPrice, bool window = true, bool reserved = false)
        : Seat(number, seatPrice, reserved), isWindow(window) {}

    Business(string number)
        : Seat(number, 350.00, false), isWindow(true) {}

    string getSeatClass() const override { return "Business"; }

    bool getIsWindow() const { return isWindow; }
};

class Economy : public Seat {
private:
    bool isAisle;

public:
    Economy(string number, double seatPrice, bool aisle = false, bool reserved = false)
        : Seat(number, seatPrice, reserved), isAisle(aisle) {}

    Economy(string number) 
        : Seat(number, 120.00, false), isAisle(false) {}

    string getSeatClass() const override { return "Economy"; }

    bool getIsAisle() const { return isAisle; }
};

class Ticket {
private:
    int ticketID;
    string passengerName;
    string passportNumber;
    string seatNumber;
    double totalFare;
    static int TicketsCount;

public:
    Ticket(int id, string name, string passport, string seat, double fare): 
        ticketID(id), passengerName(name), passportNumber(passport), seatNumber(seat), totalFare(fare) {
        TicketsCount++;
    }

    ~Ticket() {
        if (TicketsCount > 0) {
            TicketsCount--;
        }
    }

    Ticket(const Ticket& other): 
        ticketID(other.ticketID), passengerName(other.passengerName), passportNumber(other.passportNumber),
        seatNumber(other.seatNumber), totalFare(other.totalFare) {
        TicketsCount++;
    }

    int getTicketID() const { return ticketID; }
    string getPassengerName() const { return passengerName; }
    string getPassportNumber() const { return passportNumber; }
    string getSeatNumber() const { return seatNumber; }
    double getTotalFare() const { return totalFare; }

    static int getTicketsCount() { return TicketsCount; }

    void displayTicket() const {
        cout << left << setw(12) << ticketID
             << setw(20) << passengerName
             << setw(16) << passportNumber
             << setw(12) << seatNumber
             << "$" << fixed << setprecision(2) << totalFare << endl;
    }
};

int Ticket::TicketsCount = 0;

string toUpperString(string str) {
    for (char &c : str) c = toupper(c);
    return str;
}

bool isValidName(const string& name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!isalpha(c) && c != ' ') return false;
    }
    return true;
}

bool isValidPassport(const string& passport) {
    if (passport.empty()) return false;
    for (char c : passport) {
        if (!isdigit(c)) return false;
    }
    return true;
}

class FlightSystem {
private:
    vector<Seat*> seats; 
    vector<Ticket> tickets;
    int nextTicketID;

public:
    FlightSystem() : nextTicketID(5001) {}

    ~FlightSystem() {
        clearSeats();
    }

    void clearSeats() {
        for (Seat* s : seats) {
            delete s;
        }
        seats.clear();
    }

    void addSeat(Seat* seatPtr) {
        if (seatPtr) {
            seats.push_back(seatPtr);
        }
    }

    void loadDataFromFile(const string& filename) {
        ifstream inFile(filename);
        if (!inFile) {
            return;
            
        }

        int numSeats = 0;
        if (!(inFile >> numSeats) || numSeats <= 0) {
            inFile.close();
            return;
        }

        vector<Seat*> tempSeats;
        for (int i = 0; i < numSeats; i++) {
            string sNum, sClass;
            double price;
            bool reserved;
            if (inFile >> sNum >> sClass >> price >> reserved) {
                if (sClass == "Business") {
                    tempSeats.push_back(new Business(sNum, price, true, reserved));
                } else {
                    tempSeats.push_back(new Economy(sNum, price, false, reserved));
                }
            }
        }

        int numTickets = 0;
        vector<Ticket> tempTickets;
        if (inFile >> numTickets) {
            for (int i = 0; i < numTickets; i++) {
                int id;
                string seat, passport, name;
                double fare;
                if (inFile >> id >> seat >> fare >> passport) {
                    inFile.ignore();
                    getline(inFile, name);
                    tempTickets.push_back(Ticket(id, name, passport, seat, fare));
                    if (id >= nextTicketID) nextTicketID = id + 1;
                }
            }
        }
        inFile.close();


        if (!tempSeats.empty()) {
            clearSeats();
            seats = tempSeats;
            tickets = tempTickets;
        }
    }

    void saveDataToFile(const string& filename) const {
        ofstream outFile(filename);
        if (!outFile) {
            throw SystemException("Error: Could not write to file: " + filename);
        }

        outFile << seats.size() << "\n";
        for (const auto& seat : seats) {
            outFile << seat->getSeatNumber() << " "
                    << seat->getSeatClass() << " "
                    << seat->getPrice() << " "
                    << seat->getIsReserved() << "\n";
        }

        outFile << tickets.size() << "\n";
        for (const auto& ticket : tickets) {
            outFile << ticket.getTicketID() << " "
                    << ticket.getSeatNumber() << " "
                    << ticket.getTotalFare() << " "
                    << ticket.getPassportNumber() << "\n"
                    << ticket.getPassengerName() << "\n";
        }

        outFile.close();
    }

    void showSeatMapScreen() const {
        system("cls");
        cout << "--- FLIGHT SEAT MAP ---\n\n";
        cout << left << setw(12) << "Seat No." << setw(15) << "Class" << setw(15) << "Price" << "Status\n";
        cout << "---------------------------------------------------\n";
        
        for (const auto& seat : seats) {
            seat->displaySeat(); 
        }
        cout << "\n";
        system("pause");
    }

    void showReservationScreen() {
        system("cls");
        cout << "--- NEW PASSENGER RESERVATION ---\n\n";

        cout << left << setw(12) << "Seat No." << setw(15) << "Class" << setw(15) << "Price" << "Status\n";
        cout << "---------------------------------------------------\n";
        for (const auto& seat : seats) {
            seat->displaySeat();
        }

        string seatChoice, passengerName, passportNum;
        Seat* selectedSeat = nullptr;

        while (true) {
            cout << "\nEnter Seat Number to reserve (e.g., 1A): ";
            cin >> seatChoice;
            seatChoice = toUpperString(seatChoice);

            selectedSeat = nullptr;
            for (auto& seat : seats) {
                if (seat->getSeatNumber() == seatChoice) {
                    selectedSeat = seat;
                    break;
                }
            }

            try {
                if (!selectedSeat) {
                    throw SystemException("Seat " + seatChoice + " does not exist. Please try again.");
                }
                if (selectedSeat->getIsReserved()) {
                    throw SystemException("Seat " + seatChoice + " is already reserved. Please select an available seat.");
                }
                break;
            } catch (const SystemException& e) {
                cout << "[ERROR]: " << e.what() << endl;
            }
        }

        cin.ignore();

        while (true) {
            cout << "Enter Passenger Full Name: ";
            getline(cin, passengerName);

            try {
                if (!isValidName(passengerName)) {
                    throw SystemException("Invalid Name! Must contain letters and spaces only.");
                }
                break;
            } catch (const SystemException& e) {
                cout << "[ERROR]: " << e.what() << endl;
            }
        }

        while (true) {
            cout << "Enter Passport Number: ";
            cin >> passportNum;

            try {
                if (!isValidPassport(passportNum)) {
                    throw SystemException("Invalid Passport! Must contain digits only.");
                }
                break;
            } catch (const SystemException& e) {
                cout << "[ERROR]: " << e.what() << endl;
            }
        }

        double fare = selectedSeat->getPrice();
        selectedSeat->setReserved(true);

        Ticket newTicket(nextTicketID++, passengerName, passportNum, seatChoice, fare);
        tickets.push_back(newTicket);

        system("cls");
        cout << "--- RESERVATION CONFIRMED ---\n\n";
        cout << " Ticket ID       : #" << newTicket.getTicketID() << "\n";
        cout << " Passenger Name  : " << passengerName << "\n";
        cout << " Passport Number : " << passportNum << "\n";
        cout << " Assigned Seat   : " << seatChoice << " (" << selectedSeat->getSeatClass() << ")\n";
        cout << " Total Fare      : $" << fixed << setprecision(2) << fare << "\n\n";
        system("pause");
    }

    void showTicketsScreen() const {
        system("cls");
        cout << "--- ACTIVE PASSENGER LIST ---\n\n";
        if (tickets.empty()) {
            cout << " No passenger reservations found.\n";
        } else {
            cout << left << setw(12) << "Ticket ID" << setw(20) << "Passenger Name" << setw(16) << "Passport" << setw(12) << "Seat" << "Fare\n";
            cout << "-----------------------------------------------------------------------\n";
            for (const auto& ticket : tickets) {
                ticket.displayTicket();
            }
        }
        cout << "\n";
        system("pause");
    }

    void showCancellationScreen() {
        system("cls");
        cout << "--- CANCEL TICKET ---\n\n";

        if (tickets.empty()) {
            cout << " No active passenger reservations to cancel.\n\n";
            system("pause");
            return;
        }

        cout << "--- ACTIVE RESERVATIONS ---\n";
        cout << left << setw(12) << "Ticket ID" << setw(20) << "Passenger Name" << setw(16) << "Passport" << setw(12) << "Seat" << "Fare\n";
        cout << "-----------------------------------------------------------------------\n";
        for (const auto& ticket : tickets) {
            ticket.displayTicket();
        }
        cout << "-----------------------------------------------------------------------\n\n";

        string seatChoice;
        Seat* selectedSeat = nullptr;

        while (true) {
            cout << "Enter Seat Number to cancel (e.g., 1A): ";
            cin >> seatChoice;
            seatChoice = toUpperString(seatChoice);

            selectedSeat = nullptr;
            for (auto& seat : seats) {
                if (seat->getSeatNumber() == seatChoice) {
                    selectedSeat = seat;
                    break;
                }
            }

            try {
                if (!selectedSeat) {
                    throw SystemException("Error: Seat " + seatChoice + " does not exist in system.");
                }

                if (!selectedSeat->getIsReserved()) {
                    throw SystemException("Error: Seat " + seatChoice + " is not currently marked as reserved.");
                }

                auto ticketIt = tickets.end();
                for (auto it = tickets.begin(); it != tickets.end(); ++it) {
                    if (it->getSeatNumber() == seatChoice) {
                        ticketIt = it;
                        break;
                    }
                }

                if (ticketIt == tickets.end()) {
                    throw SystemException("Error: No active booking ticket found for seat " + seatChoice + ".");
                }

                selectedSeat->setReserved(false);
                tickets.erase(ticketIt);
                break;

            } catch (const SystemException& e) {
                cout << "[CANCELLATION ERROR]: " << e.what() << endl;
            }
        }

        system("cls");
        cout << "--- CANCELLATION SUCCESSFUL ---\n\n";
        cout << " Seat " << seatChoice << " is now AVAILABLE.\n\n";
        system("pause");
    }

    void showExportReportScreen(const string& filename) const {
        system("cls");
        cout << "--- EXPORT FLIGHT REPORT ---\n\n";
        saveDataToFile(filename);
        cout << " Report successfully saved to '" << filename << "'.\n\n";
        system("pause");
    }

    void showExitScreen(const string& filename) const {
        system("cls");
        saveDataToFile(filename);
        cout << "--- SYSTEM EXIT ---\n\n";
        cout << " Flight data saved successfully to '" << filename << "'.\n";
        cout << " System session ended. Thank you!\n";
    }
};

int main() {
    FlightSystem flight;

    flight.addSeat(new Business("1A", 350.00, true));
    flight.addSeat(new Business("1B", 350.00, false));
    flight.addSeat(new Economy("2A", 120.00, true));
    flight.addSeat(new Economy("2B", 120.00, false));
    flight.addSeat(new Economy("3A")); 
    flight.addSeat(new Economy("3B"));

    try {
        flight.loadDataFromFile("flight_data.txt");
    } catch (const SystemException&) {}

    string inputChoice;
    int choice = 0;

    do {
        system("cls");
        cout << "=== FLIGHT RESERVATION SYSTEM ===\n";
        cout << " [System Stats: " << Seat::getSeatsCount() << " Active Seats | " 
             << Ticket::getTicketsCount() << " Active Tickets]\n";
        cout << "-----------------------------------\n";
        cout << " 1. View Seat Map\n";
        cout << " 2. Reserve a Seat\n";
        cout << " 3. View Active Passenger List\n";
        cout << " 4. Cancel a Ticket Reservation\n";
        cout << " 5. Export Flight Report to File\n";
        cout << " 6. Save & Exit System\n";
        cout << "-----------------------------------\n";

        while (true) {
            cout << " Selection (1-6): ";
            cin >> inputChoice;

            bool isNumeric = !inputChoice.empty();
            for (char c : inputChoice) {
                if (!isdigit(c)) {
                    isNumeric = false;
                    break;
                }
            }

            try {
                if (!isNumeric) {
                    throw SystemException("Invalid input! Choice must be a number (1-6).");
                }

                choice = stoi(inputChoice);

                if (choice < 1 || choice > 6) {
                    throw SystemException("Invalid option! Please choose a number between 1 and 6.");
                }

                break;
            } 
            catch (const SystemException& e) {
                cout << "[ERROR]: " << e.what() << endl;
            }
        }

        try {
            switch (choice) {
                case 1: flight.showSeatMapScreen(); break;
                case 2: flight.showReservationScreen(); break;
                case 3: flight.showTicketsScreen(); break;
                case 4: flight.showCancellationScreen(); break;
                case 5: flight.showExportReportScreen("flight_report.txt"); break;
                case 6: flight.showExitScreen("flight_data.txt"); break;
            }
        } 
        catch (const SystemException& e) {
            system("cls");
            cout << "\n[FLIGHT SYSTEM ERROR]: " << e.what() << "\n\n";
            system("pause");
        }
        catch (const std::exception& e) {
            system("cls");
            cout << "\n[STANDARD EXCEPTION]: " << e.what() << "\n\n";
            system("pause");
        }
		
    } while (choice != 6);

    return 0;
}