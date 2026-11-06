// The kingdom advisor and its dialogs (advisors_kingdom.c): their draws and buttons, and the texts of the kingdom
// advisor's group.

#ifndef WINDOW_ADVISORS_KINGDOM_H
#define WINDOW_ADVISORS_KINGDOM_H

#include <stdint.h>

enum kingdom_advisor_text {
    KINGDOM_TEXT_RATING = 0,
    KINGDOM_TEXT_SAVINGS_OF = 1,
    KINGDOM_TEXT_GIVE_TO_CITY = 2,
    KINGDOM_TEXT_DEBEN_PER_MONTH = 3,
    KINGDOM_TEXT_SALARY_OF_RANK_FIRST = 4,
    KINGDOM_TEXT_SET_SALARY_LEVEL = 15,
    KINGDOM_TEXT_GIVE_MONEY_TO_CITY = 16,
    KINGDOM_TEXT_DONATION_IS = 17,
    KINGDOM_TEXT_GIVE_MONEY = 18,
    KINGDOM_TEXT_ALL = 19,
    KINGDOM_TEXT_NO_REQUESTS = 21,
    KINGDOM_TEXT_RATING_OPINION_FIRST = 22,
    KINGDOM_TEXT_IN_STORAGE_YARDS = 43,
    KINGDOM_TEXT_IN_CITY_TREASURY = 44,
    KINGDOM_TEXT_CLICK_TO_DISPATCH = 47,
    KINGDOM_TEXT_UNABLE_TO_FULFILL = 48,
    KINGDOM_TEXT_SEND_GIFT = 49,
    KINGDOM_TEXT_TIME_SINCE_LAST_GIFT = 50,
    KINGDOM_TEXT_MODEST_GIFT_FIRST = 51,
    KINGDOM_TEXT_GENEROUS_GIFT_FIRST = 55,
    KINGDOM_TEXT_LAVISH_GIFT_FIRST = 59,
    KINGDOM_TEXT_GIFT_SIZE_FIRST = 63,
    KINGDOM_TEXT_DISPATCH_GIFT_FIRST = 66,
    KINGDOM_TEXT_GIVE_TO_EGYPT = 69,
    KINGDOM_TEXT_NOT_ENOUGH_SAVINGS = 70,
    KINGDOM_TEXT_SALARY_ABOVE_RANK = 71,
    KINGDOM_TEXT_DISPATCH_TROOPS_TO = 72,
    KINGDOM_TEXT_SMALL_FORCE_ATTACKS_IN = 73,
    KINGDOM_TEXT_AVERAGE_FORCE_ATTACKS_IN = 74,
    KINGDOM_TEXT_LARGE_FORCE_ATTACKS_IN = 75,
    KINGDOM_TEXT_FREE_TO_SET_SALARY = 76,
    KINGDOM_TEXT_SALARY_FORBIDDEN = 77,
    KINGDOM_TEXT_NEED_MANSION = 78,
    KINGDOM_TEXT_DEBENS_LOST_TO_THEFT = 79,
    KINGDOM_TEXT_DISPATCH_WATERBORNE_TROOPS_TO = 80,
    KINGDOM_TEXT_POLITICAL_OVERSEER_FOR = 81,
};

enum message_button_text {
    MESSAGE_BUTTON_TEXT_TO_COMPLY = 2,
};

void window_advisor_kingdom_draw_background(void);
void window_go_burial_provisions(void);
void window_advisor_kingdom_draw_foreground(void);
void window_set_salary_draw(void);
void window_donate_to_city_draw_background(void);
void window_donate_to_city_draw_foreground(void);
void window_send_gift_draw_background(void);
void window_send_gift_draw_foreground(void);
void burial_provisions_all(void);
void burial_provisions_more(void);
void burial_provisions_less(void);
void burial_provisions_back(void);
void burial_provisions_send(void);
void advisor_donation_arrow(void);
void request_popup_answer(void);
void window_burial_provisions_draw_background(void);
void window_burial_provisions_draw_foreground(void);
void button_donate_to_city(void);
void button_set_salary(void);
void button_send_gift(void);
void button_request(void);
void button_salary_cancel(void);
void button_salary_rank(void);
void button_donate_give(void);
void button_donate_amount(void);
void button_gift_select(void);
void button_gift_send(void);

#endif
