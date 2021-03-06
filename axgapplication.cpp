#include "Wt/WEnvironment"
#include "Wt/WText"
#include "Wt/WLineEdit"
#include "Wt/WContainerWidget"
#include "Wt/WPushButton"
#include "Wt/WJavaScript"
#include "Wt/WMessageBox"
#include "Wt/WLogger"
#include "Wt/WServer"
#include <boost/lexical_cast.hpp>
#include <boost/functional.hpp>
#include <sstream>


#include "Ui/dialogwindowholder.h"
#include "axgapplication.h"
#include "logger.h"
#include "ggwrapper.h"
#include "loginresultevent.h"
#include "Ui/loginwindow.h"
#include "alivechecker.h"
#include "Ui/dialogwindow.h"
#include "messageevent.h"
#include "TypingNotificationEvent.h"
#include "ContactImportEvent.h"
#include "Ui/contactlist.h"
#include "Ui/contactwindow.h"

#include "History/historymanager.h"
#include "sitetitleupdater.h"
using namespace Wt;


#define LAYOUT_TEST1
AxgApplication::AxgApplication(const Wt::WEnvironment& env)
  : WApplication(env),
    mpWrapper(new GGWrapper()),
    #ifndef LAYOUT_TEST
    mpLoginWindow(new LoginWindow(root())),
    #else
    mpDialogWindowHolder(new DialogWindowHolder(root())),
    mpContactWindow(new ContactWindow(root())),
    #endif

    mpAliveChecker(new AliveChecker(root())),

    mpWindowUnloadSignal(new Wt::JSignal<void>(this,"WindowUnloadSignal")),
    mpWindowFocusChange(new Wt::JSignal<std::string>(this,"WindowFocusChangeSignal")),
    mpSiteTitleUpdater(new SiteTitleUpdater(this))
{
    enableUpdates();
    setTitle("AxG");
    initConnections();
    initJSScripts();
#ifdef LAYOUT_TEST
    mpContactWindow->windowOpenRequest().connect(mpDialogWindowHolder,&DialogWindowHolder::openDialogWindowRequest);
#endif


    this->useStyleSheet("style/style.css");
    std::stringstream ss;


    mpSiteTitleUpdater->titleUpdateRequest().connect(this,&AxgApplication::onTitleUpdateRequest);
}
AxgApplication::~AxgApplication()
{
    delete mpWindowUnloadSignal;
    delete mpWrapper;
}
void AxgApplication::onTitleUpdateRequest(std::string newTitle)
{

    doJavaScript("window.blinkOn = true");
    if(newTitle.empty())
    {
        newTitle = "Axg";
    }
    else
    {
       newTitle = "Axg " + newTitle;
    }
    if(newTitle != title())
    {
        setTitle(Wt::WString::fromUTF8(newTitle));
        triggerUpdate();
    }
}

void AxgApplication::initJSScripts()
{
    requireJQuery("scripts/jquery-2.0.3.min.js");
    require("scripts/jquery-ui.min.js");
    this->useStyleSheet("style/jquery-ui.css");
    std::stringstream ss;

    ss << "window.onbeforeunload =function(){" <<mpWindowUnloadSignal->createCall() << "};";
    ss << "$(window).on(\"blur focus\",function(e) {";
    ss << "var prevType = $(this).data(\"prevFocusChangeType\");";
    ss << "if( prevType != e.type){ " << mpWindowFocusChange->createCall("e.type") << "}";
    ss << "$(this).data(\"prevFocusChangeType\",e.type);";
    ss << "if(e.type == 'blur') $dragging = null;";
    ss <<"})";

    //dAutoJavaScript(ss.str());
    declareJavaScriptFunction("InitJSSScripts",ss.str());

    require("./scripts/globals.js");
}


void AxgApplication::initConnections()
{
    mpWindowUnloadSignal->connect(this,&AxgApplication::onWindowUnload);
    mpWrapper->eventSignal().connect(this,&AxgApplication::onEvent);
    #ifndef LAYOUT_TEST
    mpLoginWindow->loginSignal().connect(mpWrapper,&GGWrapper::connect);
    #endif
    mpAliveChecker->died().connect(this,&AxgApplication::onQuitRequested);
    mpWindowFocusChange->connect(this,&AxgApplication::onWindowFocusChange);
}


void AxgApplication::onWindowUnload()
{
    //Logger::log("Unload called \n");
    quit();

}
void AxgApplication::onWindowFocusChange(std::string newState)
{
    if(newState == "focus")
    {
        mpSiteTitleUpdater->siteGainedFocus();
        doJavaScript("ctrlPressed=false");
    }
    else
        mpSiteTitleUpdater->siteLostFocus();

}

void AxgApplication::finalize()
{
    WApplication::finalize();
}

void AxgApplication::onQuitRequested()
{
    Logger::log("Quit Requested.. pushing onto UI");
    Wt::WServer::instance()->post(sessionId(),boost::bind(&AxgApplication::quit,this));
}

//called on wrong thread has to be pushed to UI.
void AxgApplication::onEvent(boost::shared_ptr<Event> event)
{
    //push Event To UI
    Logger::log("Got Event on Worker Thread");
    Wt::WServer::instance()->post(sessionId(),
                                  boost::bind<void>(
                                      [this,event]()
    {
        onEventUIThread(event);

    }));

}
void AxgApplication::onEventUIThread(boost::shared_ptr<Event> event)
{
    //auto lock = UpdateLock(this);
    switch(event->getType())
    {
        case Event::LoginResult:
            onLoginResult(event);
            break;
        case Event::MessageRcv:
            onMessageRcv(event);
            break;
        case Event::ContactImport:
            onContactImport(event);
            break;
        case Event::TypingNotification:
            onTypingNotification(event);
            break;
    }
    triggerUpdate();
}
void AxgApplication::onContactImport(boost::shared_ptr<Event> event)
{
    ContactImportEvent *contactsEvent = static_cast<ContactImportEvent*>(event.get());
    this->mpContactWindow->contactsReceived(contactsEvent);
}

void AxgApplication::onMessageRcv(boost::shared_ptr<Event> event)
{
    MessageEvent *msgEvent = static_cast<MessageEvent*>(event.get());
    this->mpDialogWindowHolder->messageReceived(msgEvent);
}

void AxgApplication::onLoginResult(boost::shared_ptr<Event> event)
{
    LoginResultEvent* loginResultEvent = static_cast<LoginResultEvent*>(event.get());
    if(loginResultEvent->wasLoginSuccesfull)
    {
        root()->removeWidget(mpLoginWindow);
        delete mpLoginWindow;
        mpDialogWindowHolder = new DialogWindowHolder(loginResultEvent->uin,this->root());
        HistoryManager::informAboutLogin(loginResultEvent->uin);
        mpContactWindow = new ContactWindow(root());
        mpContactWindow->windowOpenRequest().connect(mpDialogWindowHolder,&DialogWindowHolder::openDialogWindowRequest);
        mpContactWindow->windowOpenRequestForceActivate().connect(mpDialogWindowHolder,&DialogWindowHolder::openDialogWindowAndActivateRequest);
        mpDialogWindowHolder->newContactInfoRequest().connect(mpContactWindow,&ContactWindow::onNewContactInfoRequest);
        mpDialogWindowHolder->sendMessageRequest().connect(mpWrapper,&GGWrapper::sendMessage);
        mpDialogWindowHolder->sendTypingNotificationRequest().connect(mpWrapper,&GGWrapper::sendTypingNotification);
        mpDialogWindowHolder->newUnreadMessage().connect(mpSiteTitleUpdater,&SiteTitleUpdater::newUnreadMessage);
        mpDialogWindowHolder->messagesRead().connect(mpSiteTitleUpdater,&SiteTitleUpdater::messagesRead);
    }
    else
    {
        mpLoginWindow->reset();
        doJavaScript("alert('Failed to login NYGGA!');");
    }
}

void AxgApplication::onTypingNotification(boost::shared_ptr<Event> event)
{
    TypingNotificationEvent* typingNotificationEvent = static_cast<TypingNotificationEvent*>(event.get());
    this->mpDialogWindowHolder->typingNotificationReceived(typingNotificationEvent);
}
