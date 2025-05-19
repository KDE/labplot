#ifndef SCRIPTRUNTIME_H
#define SCRIPTRUNTIME_H

#include <QObject>
#include <QIcon>

class Script;

class ScriptRuntime : public QObject {
    Q_OBJECT

public:
    explicit ScriptRuntime(const QString&, Script*);
    virtual ~ScriptRuntime() = default;
    
    const QString lang;
    int errorLine() const;

    virtual bool init() = 0;
    virtual bool cancel() = 0;
    virtual bool exec(const QString&) = 0;
    virtual QIcon icon() = 0;

protected:
    const QString m_name;
    int m_errorLine{-1};
    bool isCancellable{false};

Q_SIGNALS:
    void writeOutput(bool, const QString&);
};

#endif