#include <iostream>
#include <httplib.h>
#include <cookie.h>

using namespace httplib;

static std::string generate_random_string(int cookie_size)
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

httplib::Server::Handler loginCheck(httplib::Server::Handler next);
void loginHandler(const Request& req, Response& res);
void homeHandler(const Request& req, Response& res);
void showHandler(const Request& req, Response& res);
void getToHandler(const Request& req, Response& res);
void logoutHandler(const Request& req, Response& res);

int main()
{
    Server server;
    server.Get("/", loginCheck(homeHandler));
    server.Post("/", loginCheck(homeHandler));
    server.Get("/login", loginCheck(loginHandler));
    server.Post("/login", loginCheck(loginHandler));
    server.Get("/show-short-link", loginCheck(showHandler));
    server.Get("/get-to", getToHandler);
    server.Get("/logout", loginCheck(logoutHandler));

    server.listen("0.0.0.0", 8080);
}

static bool is_session_exist(const Request& req)
{
    auto cookie = Cookie::get_cookie(req, "user_cookie");

    if (cookie.name == "") return false;

    if (sessions.count(cookie.value) == 0) return false;

    return true;
}

httplib::Server::Handler loginCheck(httplib::Server::Handler next)
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
			<label for="login"></label>
			<input type="text" autocomplete="off" placeholder="Ââåäèòå ëîãèí" name="login" required>
			<label for="password"></label>
			<input type="password" autocomplete="off" placeholder="Ââåäèòå ïàðîëü" name="password" required>
			<button class="btn-new" type="submit">Âõîä</button>
		    </div>
		</form>
            </body>
            </html>
    	)#";

        res.set_content(page, "text/html");
    }

    if (req.method == "POST")
    {
        const std::string correct_password = "15123415";

        auto login = req.has_param("login") ? req.get_param_value("login") : "";
        auto password = req.has_param("password") ? req.get_param_value("password") : "";

        if (login != "" && password == correct_password)
        {
            Cookie cookie;
            cookie.name = "user_cookie";
            cookie.value = generate_random_string(16);
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

std::map<std::string, std::string> shortLinks;

void homeHandler(const Request& req, Response& res)
{
    auto cookie = Cookie::get_cookie(req, "user_cookie");
    auto& session = sessions[cookie.value];

    if (req.method == "GET")
    {
        std::string page = u8R"#(
            <!DOCTYPE html>
            <html>
                <head>
                    <meta charset="UTF-8">
                    <title>Home</title>
                    <style>
                        .main-box {
                            border: 1px solid;
                            border-radius: 3px;
                            position: absolute;
                            top: 50 %;
                            left: 50 %;
                            transform: translate(-50 %, -50 %);
                            padding: 10px;
                        }
                        .inp-str {
                            border: 1px solid;
                            border-radius: 2px;
                        }
                        .sbmt {
                            border: 1px;
                            border-radius: 3px;
                            color: #FFF;
                            background-color: #0000FF;
                        }
                    </style>
                </head>
                <body>
                    <div class="main-box">
                        <form action="/" method="post">
                            <label for="long-link">
                                <input class="inp-str" type="url" name="long-link" placeholder="Enter long URL" autocomplete="off">
                                <button class="sbmt" type="submit">Shorten</button>
                            </label>
                        </form>
                    </div>
                </body>
            </html>
        )#";
        
        res.set_content(page, "text/html");
    }
    
    if (req.method == "POST")
    {
        auto long_link = req.has_param("long-link") ? req.get_param_value("long-link") : "";
        auto short_link_id = generate_random_string(8);
        shortLinks[short_link_id] = long_link;
        res.set_redirect("/show-short-link?id=" + short_link_id);
    }
}

void showHandler(const Request& req, Response& res)
{
    std::string page;
    auto id = req.get_param_value("id");
    page += u8"<p>26.83.106.113:8080/get-to?id=" + id;

    res.set_content(page, "text/html");
}

void getToHandler(const Request& req, Response& res)
{
    if (req.method == "GET")
    {
        std::string id = req.get_param_value("id");
        std::string link = shortLinks[id];
        std::cout << link << std::endl;
        res.set_redirect(link.c_str());
    }
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
