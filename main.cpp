#include <iostream>
#include <httplib.h>
#include <cookie.h>

using namespace httplib;

std::string generateCookie(int cookie_size)
{
    std::string letters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string new_cookie = "";

    std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());

    for (int i = 0; i < cookie_size; i++)
    {
        std::uniform_int_distribution<int> distribution(0, letters.length() - 1);
        int randomIndex = distribution(generator);
        new_cookie += letters[randomIndex];
    }

    return new_cookie;
}

using session_data = std::map<std::string, std::string>;
std::map<std::string, session_data> sessions;

httplib::Server::Handler login—heck(httplib::Server::Handler next);
void registerHandler(const Request& req, Response& res);
void loginHandler(const Request& req, Response& res);
void homeHandler(const Request& req, Response& res);
void logoutHandler(const Request& req, Response& res);

int main()
{
    Server server;
    server.Get("/", login—heck(homeHandler));
    server.Get("/login", login—heck(loginHandler));
    server.Post("/login", login—heck(loginHandler));
    server.Get("/logout", login—heck(logoutHandler));

    server.listen("0.0.0.0", 8080);
}

static bool is_session_exist(const Request& req)
{
    auto cookie = Cookie::get_cookie(req, "user_cookie");

    if (cookie.name == "") return false;

    if (sessions.count(cookie.value) == 0) return false;

    return true;
}

httplib::Server::Handler login—heck(httplib::Server::Handler next)
{
    return [next](const Request& req, Response& res)
        {
            if (!is_session_exist(req) && req.path != "/login")
            {
                res.set_redirect("/login");
                return;
            }

            if (is_session_exist(req) && req.path == "/login")
            {
                res.set_redirect("/");
                return;
            }

            next(req, res);
        };
}

void loginHandler(const Request& req, Response& res)
{
    if (req.method == "GET")
    {
        std::string page = u8R"#(
            <!DOCTYPE html>
            <html>
            <head>
		        <meta charset="UTF-8">
		        <meta name="viewport" content="width=device-width, initial-scale=1.0">
		        <title>Login page</title>
                <style>
                    body {
                        background-color: #f0f0f0;
                        font-family: Arial, sans-serif;
                    }
                    .mainContainer {
                        position: absolute;
                        top: 50%;
                        left: 50%;
                        transform: translate(-50%, -50%);
                        padding: 20px;
                        background-color: #fff;
                        box-shadow: 0px 0px 10px rgba(0,0,0,0.1);
                        border-radius: 10px;
                    }
                    label {
                        display: inline-block;
                        margin-bottom: 5px;
                    }
                    input[type="text"], input[type="password"] {
                        width: 94%;
                        padding: 10px;
                        margin-bottom: 10px;
                        border-radius: 5px;
                        border: 1px solid #ccc;
                    }
                    .btn-new {
                        width: 60px;
                        height: 35px;
                        border-radius: 10px;
                        color: white;
                        transition: .2s linear;
                        background: #0B63F6;
                    }
                    .btn-new:hover {
                        box-shadow: 0 0 0 2px white, 0 0 0 4px #3C82F8;
                    }
                </style>
            </head>
            <body>
		        <form method="post" action="/login" accept-charset="UTF-8">
				    <div class="mainContainer">
						<label for="surname"></label>
						<input type="text" autocomplete="off" placeholder="¬‚Â‰ËÚÂ Ù‡ÏËÎË˛" name="surname" required>
						<label for="password"></label>
                        <input type="password" autocomplete="off" placeholder="œ‡ÓÎ¸ ‡‰ÏËÌ‡" name="password" required>
						<button class="btn-new" type="submit">¬ıÓ‰</button>
				    </div>
		        </form>
            </body>
            </html>)#";

        res.set_content(page, "text/html");
    }

    if (req.method == "POST")
    {
        const std::string correct_password = "15123415";

        auto login = req.has_param("login") ? req.get_param_value("login") : "";
        auto password = req.has_param("password") ? req.get_param_value("password") : "";

        if (is_registered(login) && password == correct_password)
        {
            Cookie cookie;
            cookie.name = "user_cookie";
            cookie.value = generateCookie(16);
            cookie.path = "/";
            cookie.maxAge = 3600;
            cookie.httpOnly = true;
            cookie.sameSite = Cookie::SameSiteLaxMode;

            sessions[cookie.value]["login"] = login;

            Cookie::set_cookie(res, cookie);

            std::cout << login << " has been connected with cookie: " << cookie.value << std::endl;
        }

        res.set_redirect("/");
    }
}

void homeHandler(const Request& req, Response& res)
{
    auto cookie = Cookie::get_cookie(req, "user_cookie");
    auto& session = sessions[cookie.value];

    std::string page;
    
    res.set_content(page, "text/html");
}

void logoutHandler(const Request& req, Response& res)
{
    if (req.method == "GET")
    {
        auto cookie = Cookie::get_cookie(req, "user_cookie");

        std::cout << "Cookie: " << cookie.value << " now is outdated." << std::endl;

        sessions.erase(cookie.value);

        Cookie new_cookie;
        new_cookie.name = "user_cookie";
        new_cookie.value = "";
        new_cookie.maxAge = -1;

        Cookie::set_cookie(res, new_cookie);

        res.set_redirect("/login");
    }
}

void registerHandler(const Request& req, Response& res)
{

}