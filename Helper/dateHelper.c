// Eelybase - (c) 2022-24 by Joerg Plenert | https://eely.eu
int dayOfYear(string date)
{
    // Extract the year, month and the
    // day from the date string
    int year = stoi(date.substr(0, 4));
    int month = stoi(date.substr(5, 2));
    int day = stoi(date.substr(8));
 
    // If current year is a leap year and the date
    // given is after the 28th of February then
    // it must include the 29th February
    if (month > 2 && year % 4 == 0
        && (year % 100 != 0 || year % 400 == 0)) {
        ++day;
    }
 
    // Add the days in the previous months
    while (month-- > 0) {
        day = day + days[month - 1];
    }
    return day;
}