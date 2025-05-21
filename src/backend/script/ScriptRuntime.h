#ifndef SCRIPTRUNTIME_H
#define SCRIPTRUNTIME_H

#include <QObject>
#include <QIcon>

class Script;

/*!
 * This abstract class is the interface which script runtimes should
 * implement
 */
class ScriptRuntime : public QObject {
    Q_OBJECT

public:
    explicit ScriptRuntime(const QString&, Script*);
    virtual ~ScriptRuntime() = default;
    
    const QString lang;

    // Returns the line in the code incase an error occurred during execution
    int errorLine() const;

    // Initializes the script runtime. Is the first method called for every instance of
    // the script runtime. Should return true or false on success or failure
    virtual bool init() = 0;

    // Stops the current execution. Should return true or false on success or failure
    virtual bool cancel() = 0;

    // Executes the code passed as parameter. Should return true or false on success or failure
    virtual bool exec(const QString& code) = 0;

    // The icon for the script runtime
    virtual QIcon icon() = 0;

protected:
    const QString m_name;
    int m_errorLine{-1}; // Should return -1 if no error or the error line is unknown
    bool isCancellable{false}; // Should return true or false is the runtime is cancellable or not

Q_SIGNALS:
    // Writes output to the ScriptEditor output. The isErr parameter indicates
    // if the output is from stderr or from stdout
    void writeOutput(bool isErr, const QString& output);
};

#endif