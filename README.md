hurt-heal
=========

Hurt &amp; heal

Bare-bones voting process
-------------------------

* Client requests ReCaptcha challenge code
    * If client can vote, server returns a ReCaptcha challenge code (which consists only of alphanumeric characters, underscores, and hyphens)
    * If client can't vote, server returns a period
* Client sends vote, ReCaptcha challenge, and ReCaptcha solution as form data
    * If the vote has successfully been cast, server returns statistics
    * If the vote has failed (incorrect ReCaptcha solution), server returns a period

Maybe the server should generate what characters are available to be voted for using the ReCaptcha challenge code so that the client can't vote for whatever characters they want.

Sample vote
-----------
    hurt=1.2.3.&heal=4.5.&cc=123abc&cs=Hello%20world
