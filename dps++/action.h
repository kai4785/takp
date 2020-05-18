#ifndef ACTION_H
#define ACTION_H

#include <string_view>
#include <cstdint>

class Action
{
public:
    enum Type
    {
        UNKNOWN,
        CHAT,
        MELEE,
        MAGIC,
        HEAL,
        CRIT,
        CRIP,
        HOLYBLADE,
        DEATH,
        ZONE
    };
    Action(const std::string_view& message)
        :m_type(UNKNOWN)
        ,m_damage(0)
    {
        parse(message);
    }
    void parse(const std::string_view& message);

    const Type type() const { return m_type; }
    const std::string_view source() const { return m_source; }
    const std::string_view target() const { return m_target; }
    const std::string_view verb() const { return m_verb; }
    const int64_t damage() const { return m_damage; }
    const std::string_view message() const { return m_message; }

private:
    void parseDamage(std::string_view& message);
    enum Type m_type;
    std::string_view m_source;
    std::string_view m_target;
    std::string_view m_verb;
    int64_t m_damage;
    std::string_view m_message; // Used for debugging
};

#endif // ACTION_H
