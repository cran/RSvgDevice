\name{devSVG}
\title{A SVG Graphics Driver}
\usage{
devSVG(file = "Rplots.svg", width = 5, height = 4,
       bg = "white", fg = "black", onefile=TRUE, xmlHeader=TRUE)
}
\alias{devSVG}
\arguments{
  \item{file}{the file where output will appear.}
  \item{width}{The width of the plot in inches.}
  \item{height}{the height of the plot in inches.}
  \item{bg}{the background color for the plot.}
  \item{fg}{the foreground color for the plot.}
  \item{onefile}{merge plot calls into onefile or separate them to separate pages.}
  \item{xmlHeader}{Print XML header or not.}
}
\description{
  This function produces graphics suitable teh current w3 svg XML standard
}
\details{
  This driver currently does not have any font metric information, 
  so the use of \code{\link{plotmath}} is not supported.

  The driver output is currently NOT specifying a DOCTYPE DTD
}
\references{
  www Consortum  
  \emph{W3C Scalable Vector Graphics (SVG)}
  http://www.w3.org/Graphics/SVG/Overview.htm8
}
\author{
  This driver was written by T Jake Luciani
  \email{jakeluciani@yahoo.com}
}
\seealso{
  \code{\link{pictex}}, \code{\link{postscript}}, \code{\link{Devices}}.
}
\examples{
devSVG()
plot(1:11,(-5:5)^2, type='b', main="Simple Example Plot")
dev.off()
}
\keyword{device}