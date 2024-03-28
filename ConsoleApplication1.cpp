#include <iostream>
#include <random>
#include <vector>
#include <algorithm>

const int TOTAL_YEARS = 200;
const int MINIMUM_AGE_QUAESTOR = 30;
const int MINIMUM_AGE_AEDILE = 36;
const int MINIMUM_AGE_PRAETOR = 39;
const int MINIMUM_AGE_CONSUL = 42;
const int MINIMUM_SERVICE_AEDILE = 2;
const int MINIMUM_SERVICE_PRAETOR = 2;
const int MINIMUM_SERVICE_CONSUL = 2;
const int YEARS_BETWEEN_CONSUL_REELECTION = 10;
const int QUAESTORES_PER_YEAR = 20;
const int AEDILES_PER_YEAR = 10;
const int PRAETORS_PER_YEAR = 8;
const int CONSULS_PER_YEAR = 2;
const int STARTING_PSI = 100;
const int UNFILLED_POSITION_PENALTY = -5;
const int CONSECUTIVE_CONSUL_REELECTION_PENALTY = -10;
const int ANNUAL_INFLUX_MEAN = 15;
const int ANNUAL_INFLUX_STD_DEV = 5;
const int LIFE_EXPECTANCY_MEAN = 55;
const int LIFE_EXPECTANCY_STD_DEV = 10;


class Politician {
public:
    Politician(int age) : age(age) {}
    int getAge() const { return age; }
private:
    int age;
};


class Simulation {
public:
    Simulation() : psi(STARTING_PSI), year(0) {
        politicians.reserve(QUAESTORES_PER_YEAR + AEDILES_PER_YEAR + PRAETORS_PER_YEAR + CONSULS_PER_YEAR);
        for (int i = 0; i < QUAESTORES_PER_YEAR; ++i) {
            politicians.emplace_back(MINIMUM_AGE_QUAESTOR);
        }
        for (int i = 0; i < AEDILES_PER_YEAR; ++i) {
            politicians.emplace_back(MINIMUM_AGE_AEDILE);
        }
        for (int i = 0; i < PRAETORS_PER_YEAR; ++i) {
            politicians.emplace_back(MINIMUM_AGE_PRAETOR);
        }
        for (int i = 0; i < CONSULS_PER_YEAR; ++i) {
            politicians.emplace_back(MINIMUM_AGE_CONSUL);
        }
    }

    void simulateYear(std::mt19937& gen, std::normal_distribution<>& lifeExpectancyDist) {
        std::shuffle(politicians.begin(), politicians.end(), gen); // Randomize the order of politicians
        for (auto& politician : politicians) {
            politician = Politician(politician.getAge() + 1); // Increment age
        }

        // Remove those who surpass life expectancy
        politicians.erase(std::remove_if(politicians.begin(), politicians.end(),
            [&lifeExpectancyDist, &gen](const Politician& p) { return p.getAge() > lifeExpectancyDist(gen); }), politicians.end());

        // Update PSI based on office fill rates and re-election penalties
        psi += UNFILLED_POSITION_PENALTY * (QUAESTORES_PER_YEAR - QUAESTORES_PER_YEAR) +
            UNFILLED_POSITION_PENALTY * (AEDILES_PER_YEAR - AEDILES_PER_YEAR) +
            UNFILLED_POSITION_PENALTY * (PRAETORS_PER_YEAR - PRAETORS_PER_YEAR) +
            UNFILLED_POSITION_PENALTY * (CONSULS_PER_YEAR - CONSULS_PER_YEAR);

        if (year % YEARS_BETWEEN_CONSUL_REELECTION == 0) {
            psi += CONSECUTIVE_CONSUL_REELECTION_PENALTY * (CONSULS_PER_YEAR - CONSULS_PER_YEAR);
        }
    }

    void run() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> influxDist(ANNUAL_INFLUX_MEAN, ANNUAL_INFLUX_STD_DEV);
        std::normal_distribution<> lifeExpectancyDist(LIFE_EXPECTANCY_MEAN, LIFE_EXPECTANCY_STD_DEV);

        for (year = 0; year < TOTAL_YEARS; ++year) {
            int influx = static_cast<int>(influxDist(gen));
            for (int i = 0; i < influx; ++i) {
                politicians.emplace_back(MINIMUM_AGE_QUAESTOR); 
            }

            simulateYear(gen, lifeExpectancyDist);
        }
    }

    int getFinalPSI() const { return psi; }

    void calculateAgeDistribution() {
        std::vector<int> quaestorAges;
        std::vector<int> aedileAges;
        std::vector<int> praetorAges;
        std::vector<int> consulAges;

        for (const auto& politician : politicians) {
            int age = politician.getAge();
            if (age >= MINIMUM_AGE_QUAESTOR && age < MINIMUM_AGE_AEDILE) {
                quaestorAges.push_back(age);
            } else if (age >= MINIMUM_AGE_AEDILE && age < MINIMUM_AGE_PRAETOR) {
                aedileAges.push_back(age);
            } else if (age >= MINIMUM_AGE_PRAETOR && age < MINIMUM_AGE_CONSUL) {
                praetorAges.push_back(age);
            } else if (age >= MINIMUM_AGE_CONSUL) {
                consulAges.push_back(age);
            }
        }

        auto calculateSummary = [](const std::vector<int>& ages) -> std::pair<unsigned __int64, double> {
            if (ages.empty()) {
                return std::make_pair(0ULL, 0.0);
            }

            unsigned __int64 totalAges = 0;
            for (int age : ages) {
                totalAges += age;
            }
            double averageAge = static_cast<double>(totalAges) / ages.size();

            return std::make_pair(static_cast<unsigned __int64>(ages.size()), averageAge);
        };

        auto quaestorSummary = calculateSummary(quaestorAges);
        auto aedileSummary = calculateSummary(aedileAges);
        auto praetorSummary = calculateSummary(praetorAges);
        auto consulSummary = calculateSummary(consulAges);

        std::cout << "Age Distribution:" << std::endl;
        std::cout << "Quaestor: Count=" << quaestorSummary.first << ", Average Age=" << quaestorSummary.second << std::endl;
        std::cout << "Aedile: Count=" << aedileSummary.first << ", Average Age=" << aedileSummary.second << std::endl;
        std::cout << "Praetor: Count=" << praetorSummary.first << ", Average Age=" << praetorSummary.second << std::endl;
        std::cout << "Consul: Count=" << consulSummary.first << ", Average Age=" << consulSummary.second << std::endl;
    }

private:
    int psi;
    int year;
    std::vector<Politician> politicians;
};

int main() {
    Simulation simulation;
    simulation.run();

    std::cout << "End-of-Simulation PSI: " << simulation.getFinalPSI() << std::endl;

    std::cout << "Annual Fill Rate:" << std::endl;
    std::cout << "Quaestor: " << static_cast<double>(QUAESTORES_PER_YEAR) / TOTAL_YEARS * 100 << "%" << std::endl;
    std::cout << "Aedile: " << static_cast<double>(AEDILES_PER_YEAR) / TOTAL_YEARS * 100 << "%" << std::endl;
    std::cout << "Praetor: " << static_cast<double>(PRAETORS_PER_YEAR) / TOTAL_YEARS * 100 << "%" << std::endl;
    std::cout << "Consul: " << static_cast<double>(CONSULS_PER_YEAR) / TOTAL_YEARS * 100 << "%" << std::endl;


    simulation.calculateAgeDistribution();

    return 0;
}
